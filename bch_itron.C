
/*
 * bch_itron	-- Implement the Itron BCH coding used for some ERT digital "smart" meters
 * 
 *     The Itron ERT/Collector uses a "shortened" (via truncation) BCH (255, 239, T=2) code, with 16
 * bits of parity data to validate and/or error-correct its data messages.
 * 
 */
#include <map>
#include <istream>
#include <sstream>
#include <iostream>
#include <fstream>

#include <boost/fusion/include/std_pair.hpp>

#include <boost/spirit/include/classic_position_iterator.hpp>
#include <boost/spirit/include/support_multi_pass.hpp>

#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_char_class.hpp>
#include <boost/spirit/include/qi_list.hpp>
#include <boost/spirit/include/qi_optional.hpp>

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace classic = boost::spirit::classic;

using boost::optional;

#include <ezpwd/asserter>

// Djelic GPLv2+ BCH implementation from Linux kernel.  Requires "standalone" shims for user-space
// to build lib/bch.c implementation; API matches kernel.
extern "C" {
#include <linux/bch.h>
}


namespace itron {

    typedef std::map<std::string, std::string>
    			container_t;

    // 
    // Parse a sequence of test query records into a container_t:
    // 
    //    111110010101001100000100011100000000010100111100011100000001110110001000110000001101110100101010 : 
    //        {Time:2017-03-21T20:23:55.619 SCM:{ID:35489984 Type:12 Tamper:{Phy:01 Enc:00} Consumption:  343152 CRC:0xDD2A}}
    //    111110010101001100000100011100000000010100111100011100000001110110001000110001101101110100101010 : 
    //        Bad CRC
    // 
    template <typename iter_t, typename skipper_t>
    struct test_sequence
	: qi::grammar<iter_t, container_t(), skipper_t>
    {
	test_sequence()
	    : test_sequence::base_type( query )
	{
	    query			= *( pair );
	    pair			= bits >> desc;
	    bits			= +( qi::char_( "01" ));
	    desc			= qi::char_( ":" ) >> qi::lexeme[ *( qi::char_ - qi::eol )];
	}

	qi::rule<iter_t, container_t(), skipper_t> query;
	qi::rule<iter_t, std::pair<std::string, std::string>, skipper_t> pair;
	qi::rule<iter_t, std::string(), skipper_t> bits, desc;
    };

} // namespace itron


itron::container_t		parse(
				    std::istream       &input,
				    const std::string  &filename )
{
    // iterate over stream input
    typedef std::istreambuf_iterator<char>
				base_iterator_t;
    base_iterator_t		in_begin( input );

    // convert input iterator to forward iterator, usable by spirit parser
    typedef boost::spirit::multi_pass<base_iterator_t>
				forward_iterator_t;
    forward_iterator_t		fwd_begin = boost::spirit::make_default_multi_pass( in_begin );
    forward_iterator_t		fwd_end;

    // wrap forward iterator with position iterator, to record the position
    typedef classic::position_iterator2<forward_iterator_t>
				pos_iterator_t;
    pos_iterator_t		position_begin( fwd_begin, fwd_end, filename );
    pos_iterator_t		position_end;

    // prepare output
    itron::container_t		 output;

    // parse
    try {
	itron::test_sequence<pos_iterator_t, qi::space_type>
	    			parser;
	// Ensure we complete successful parse and consume all input.
	if ( ! qi::phrase_parse(
	    position_begin, position_end,                       		// iterators over input
	    parser,
	    qi::space,
	    output )) {		                                              	// records are stored into this object
	    boost::throw_exception( qi::expectation_failure<pos_iterator_t>(
	        position_begin, position_end, qi::info( "parsing failed" )));
	} else if ( position_begin != position_end ) {
	    boost::throw_exception( qi::expectation_failure<pos_iterator_t>(
	        position_begin, position_end, qi::info( "parsing incomplete" )));
	}
    } catch( const qi::expectation_failure<pos_iterator_t>& e ) {
	const classic::file_position_base<std::string>
	    &pos	= e.first.get_position();
	std::stringstream	msg;
	msg
	    << "parse error at file " << pos.file
	    << " line " << pos.line
	    << " column " << pos.column << std::endl
	    << "'" << e.first.get_currentline() << "'" << std::endl
	    << std::setw(pos.column) << " " << "^- here"
	    << " (" << output.size() << " rows parsed)";
	for ( auto &&t : output )
	    std::cout << t.first << " : " << t.second << std::endl;
	throw std::runtime_error( msg.str() );
    }

    // return result
    return output;
}


#if 1

int main()
{
    ezpwd::asserter		assert;

    // Ensure we can get the require BCH (255, 239, T=2) codec we require.  There's no way to
    // confirm that the generator polynomial is what we expect, as it is discarded by init_bch after
    // generating the internal tables.  However, I've confirmed that it is 0b10110111101100011 ==
    // 0o267543 == 0x16f63.  Strangely, when generating the BCH code using a 16-bit CRC generator,
    // other projects (namely rtl-amr) use the polynomial 0x6f63...
    struct bch_control	       *bch	= init_bch( 8, 2, 0 );
    assert.ISEQUAL( bch->m,		  8U );
    assert.ISEQUAL( bch->n,		255U );
    assert.ISEQUAL( bch->t,		  2U );
    assert.ISEQUAL( bch->ecc_bits,	 16U );

    // Iterate over a bunch of SCM messages with various errors, seeing if they can be corrected.
    // Parse records of type "11111001....0101 : <result>\n" from the file.
    std::string			filename( "bch_itron.txt" );
    std::ifstream		ifs( filename, std::ifstream::in );
    itron::container_t		tests( parse( ifs, filename ));
    for ( auto &&t : tests )
	std::cout << t.first << " : " << t.second << std::endl;

    return assert.failures ? 1 : 0;
}

#else

///////////////////////////////////////////////////////////////////////////////

int main()
{
    std::string input(
        "1:\n"
        "00:a\n"
        "01:1\n"
        "111110010101001100000100011100000000010100111100011100000001110110001000110000001101110100101010 : {Time:2017-03-21T20:23:55.619 SCM:{ID:35489984 Type:12 Tamper:{Phy:01 Enc:00} Consumption:  343152 CRC:0xDD2A}}\n"
	"111110010101001100000100011100000000010100111100011100000001110110001000110001101101110100101010 : Bad CRC\n" );

    std::string::iterator begin = input.begin();
    std::string::iterator end = input.end();

    itron::test_sequence<std::string::iterator, qi::space_type> p;
    itron::container_t m;

    if ( ! qi::phrase_parse( begin, end, p, qi::space, m )
	 || begin != end )
    {
	std::cout << "-------------------------------- \n";
	std::cout << "Parsing failed (" << m.size() << " elements parsed)\n";
	std::cout << "-------------------------------- \n";
    }
    else
    {
	std::cout << "-------------------------------- \n";
	std::cout << "Parsing succeeded, found " << m.size() << " entries:\n";
	std::cout << "---------------------------------\n";
    }
    for ( itron::container_t::iterator it = m.begin(); it != m.end(); ++it)
    {
	std::cout << it->first;
	if (! it->second.empty())
	    std::cout << ":" << it->second;
	std::cout << std::endl;
    }
    return 0;
}

#endif
