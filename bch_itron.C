
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
#include <bitset>

#include <boost/fusion/include/std_pair.hpp>

#include <boost/spirit/include/classic_position_iterator.hpp>
#include <boost/spirit/include/support_multi_pass.hpp>

#include <boost/phoenix.hpp>
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
/*
extern "C" {
#include "djelic_bch.h"
}
*/

#include <ezpwd/bch>

typedef ezpwd::BCH<255, 239, 2>	BCH_ITRON;

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

    std::cout << "Parsed " << output.size() << " Itron test records" << std::endl;

    // return result
    return output;
}


std::pair<int,std::string>	correct_SCM(
				    ezpwd::asserter    &assert,
				    BCH_ITRON	       &bch_itron,
				    const std::string  &recvd )
{
    if ( assert.ISEQUAL( recvd.size(), size_t( 96U ))) {
	std::cout
	    << assert << " " << "expected SCM of 12 bytes (96 bits), not " << recvd.size() << " bits"
	    << std::endl;
	return std::pair<int,std::string>( -1, recvd );
    }
    
    // Convert each cluster of 8 bits into a msg byte.  Any leading bits that do not consititue a full byte
    // initialize the low bits of the first byte.
    std::vector<uint8_t>	msg;
    for ( int	 		b	= recvd.size()
	      ; b > 0
	      ; b -= 8 ) {
	std::bitset<8>		byte( recvd.substr( b < 8 ? 0 : b - 8,
						    b < 8 ? b : 8 ));
	msg.insert( msg.begin(), uint8_t( byte.to_ulong() ));
    }

    // Decode the body of the message (bytes 2-12) with the BCH codec.  This should correct up to
    // bch->t (ie. 2) unknown bit errors anywhere in the message.  We have the delivered parity bits
    // in msg[10-11] at the end of the message, and we do not have the XOR of the computed/delivered
    // parity bits.
    unsigned int		errloc[96];
    int				corr	= ezpwd::decode_bch(
					      bch_itron._bch, &msg[2], 8,
					      &msg[10],	// delivered parity
					      0,	// XOR of computed/delivered parity
					      0,	// syndrome results
					      errloc);	// resultant error locations
#if defined( DEBUG ) && DEBUG > 1
    if ( corr < 0 )
	std::cout << " ; BCH decode failed" << std::endl;
    else if ( corr == 0 ) 
	std::cout << " ; BCH decode validated message as correct" << std::endl;
    else if ( corr > 0 && unsigned( corr ) < bch_itron.T )
	std::cout << " ; BCH decode corrects " << corr << " bits w/ capacity to spare" << std::endl;
    else if ( corr > 0 && unsigned( corr ) == bch_itron.T )
	std::cout << " ; BCH decode corrects " << corr << " bits at capacity (no confidence)" << std::endl;
    else if ( corr > 0 && unsigned( corr ) > bch_itron.T )
	std::cout << " ; BCH decode corrects " << corr << " bits over capacity (probably incorrect)" << std::endl;
#endif
    std::string			fixed( recvd );
    if ( corr > 0 ) {
	// Some corrections; maybe even beyond capacity?
	std::string		loctn( recvd.size(), ' ' );
	for ( int ei = 0
		  ; ei < corr
		  ; ++ei ) {
	    // Compute the byte/bit indices.  BCH-protected payload starts at 2nd byte.  The bit
	    // number offsets in the bit-string are in the reverse order of the bit numbers in the
	    // data bytes.
	    unsigned int	byte_i	= errloc[ei] / 8;
	    unsigned int	bit_i	= errloc[ei] % 8;
	    unsigned int	i	= ( 2 + byte_i ) * 8 + ( 7 - bit_i );
	    fixed[i]			= fixed[i] == '0' ? '1' : '0';  // correct the indicated bit!
	    loctn[i]			= '^';
	}
#if defined( DEBUG )
	std::cout << fixed << std::endl
		  << loctn << " (fixed " << corr << " bits)"
		  << std::endl;
#endif
    }
    // Return validity (-'ve if invalid, otherwise # of bits corrected), and the (possibly fixed)
    // string of bits (may be different than recvd)
    return std::pair<int,std::string>( corr, fixed );
}

#if 1

int main( int argc, const char **argv )
{
    ezpwd::asserter		assert;

    // Ensure we can get the require BCH (255, 239, T=2) codec we require.  There's no way to
    // confirm that the generator polynomial is what we expect, as it is discarded by init_bch after
    // generating the internal tables.  However, I've confirmed that it is 0b10110111101100011 ==
    // 0o267543 == 0x16f63.  Strangely, when generating the BCH code using a 16-bit CRC generator,
    // other projects (namely rtl-amr) use the polynomial 0x6f63...
    BCH_ITRON			bch_itron;
    assert.ISEQUAL( bch_itron._bch->m,		  8U );
    assert.ISEQUAL( bch_itron._bch->n,		255U );
    assert.ISEQUAL( bch_itron._bch->t,		  2U );
    assert.ISEQUAL( bch_itron._bch->ecc_bits,	 16U );

    // Iterate over a bunch of SCM messages with various errors, seeing if they can be corrected.
    // Parse records of type "11111001....0101 : <result>\n" from the file in the first command-line
    // arg (or bch_itron.txt, by default).  What we expect to see are a sequence of corroberated
    // error-free readings from various ERT IDs, with occasional recovered (BCH corrected) readings
    // from the same ERT IDs with consistent Consumption values; if we never see any, then something
    // is probably wrong with our decoding...
    std::string			filename( argc >= 2 ? argv[1] : "bch_itron.txt" );
    std::ifstream		ifs( filename, std::ifstream::in );
    itron::container_t		tests( parse( ifs, filename ));

    // See how many different readings we see for each ERT (ID,type) and consumption:
    // 
    //    (ID,type): consumption: ["{Time:... SCM:...", "SCM:"]
    //
    // Normally, we would expect only one; but if other stuff changes (eg. tamper bits), we could
    // see more.
    typedef std::map<
	std::pair<int,int>,		// (ID,type)
	std::map<
    	    int,			// -> Consumption
 	    std::map<
	        std::string,		//   -> Reading
	        std::map<
	          int,                  //      -> bits corrected
	          int                   //        -> Times seen
	        >
	    >
	>
     >
    			        readings_t;
    readings_t			readings;

    for ( auto &&t : tests ) {
#if defined( DEBUG )
	std::cout << "Test: " << t.first << " == " << t.second << std::endl;
#endif
	const std::string      &recvd	= t.first;
	auto			valid	= correct_SCM( assert, bch_itron, recvd );
	int			corr	= valid.first;
	if ( corr >= 0 ) {
	    // We think this is probably a valid SCM message; BCH either verified as-is, or
	    // corrected errors w/in capacity.
	    std::string	       &fixed	= valid.second;
	    assert.ISEQUAL( corr > 0, fixed != recvd, "Changes detected don't match corrections indicated" );

	    // As a base-case test, the (now) valid, fixed record should test as valid (again) without getting fixed.
	    auto		reval	= correct_SCM( assert, bch_itron, fixed );
	    if ( assert.ISTRUE( reval.first >= 0, "re-validated fixed record not longer valid!" )
	         || assert.ISEQUAL( fixed, reval.second, "re-validated fixed record was changed!" ))
	        std::cout << " ; BCH revalidation failed"  << std::endl
	    	      << fixed << " != " << std::endl
	    	      << reval.second
	    	      << std::endl;

	    // Lets decode the SCM message.  ID (26 bits) and Consumption (24 bits)
	    unsigned long	ertid	= std::bitset<26>(
	        fixed.substr( 21, 2 ) + fixed.substr( 56, 24 )).to_ulong();
	    unsigned long	erttype	= std::bitset<4>(
	        fixed.substr( 26, 4 )).to_ulong();
	    unsigned long	tampphy	= std::bitset<2>(
	        fixed.substr( 24, 2 )).to_ulong();
	    unsigned long	tampenc	= std::bitset<2>(
	        fixed.substr( 30, 2 )).to_ulong();
	    unsigned long	consump	= std::bitset<24>(
	        fixed.substr( 32, 24 )).to_ulong();
	    unsigned long	checksum= std::bitset<16>(
	        fixed.substr( 80, 16 )).to_ulong();

	    // Lets produce a message similar to this:
	    // SCM:{ID:35489984 Type:12 Tamper:{Phy:01 Enc:00} Consumption:  343152 CRC:0xDD2A}}
	    std::ostringstream decoss;
	    decoss
	        << "SCM:{ID:"		<< std::setw( 8 ) << ertid
	        << " Type:"		<< std::setw( 2 ) << erttype
	        << " Tamper:{Phy:"	<< std::setw( 2 ) << std::setfill( '0' ) << tampphy
	        << " Enc:"		<< std::setw( 2 ) << std::setfill( '0' ) << tampenc
	        << "} Consumption:"	<< std::setw( 8 ) << std::setfill( ' ' ) << std::dec << consump
	        << " CRC:0x"		<< std::setw( 4 ) << std::setfill( '0' ) << std::hex << std::uppercase << checksum
	        << "}}";
	    std::string		decode( decoss.str() );

	    // Lets see if the valid confirmed/fixed record is in the set of tests; if it is, it better be a valid SCM message!
	    bool		found	= tests.find( fixed ) != tests.end();
	    if ( corr > 0 && found )
	        std::cout << fixed << ": (corroberated, corrected)    " << decode << std::endl;
	    else if ( corr == 0 && found )
	        std::cout << fixed << ": (corroberated)               " << decode << std::endl;
	    else if ( corr > 0 ) 
	        std::cout << fixed << ": (              corrected)    " << decode << std::endl;
	    else
	        std::cout << fixed << ": (not found, but decoded)     " << decode << std::endl;

	    if ( found ) {
		// If we've recovered a message via BCH correction, and it's in the file, our
		// decoded message had better be right.  This doesn't test the BCH correction, just
		// the decoding.
	        std::string    &result	= tests[fixed];
	        if ( assert.ISTRUE( result.find( decode ) != std::string::npos )) { // matching decoded "SCM..." in result
		    std::cout
			<< assert << " ;  expected valid SCM in corrected message, not " << result
			<< std::endl;
	        }
		readings[{ertid,erttype}][consump][result][corr]++;
	    } else {
		//readings[{ertid,erttype}][consump].insert( decode );
		readings[{ertid,erttype}][consump][decode][corr]++;
	    }
	}
    }

    // Show the interesting ones we've found.  Fail unless we get at least one!
    std::cout
	<< "Observed readings from " << readings.size() << " distinct ERT ID/type:"
	<< std::endl;
    int				interesting( 0 );
    for ( auto &&r : readings ) {
	if ( r.second.size() < 2						// only 1 consumption reading from the ERT,
	     && r.second.begin()->second.size() < 2 				// and it was seen only once
	     && r.second.begin()->second.begin()->second.size() < 2		// and only one BCH correction (eg. 0/1/2)
	     && r.second.begin()->second.begin()->second.begin()->second < 2 ) {// and it wasn't seen more than once
#if ! defined( DEBUG )  // Unless DEBUG, ignore ERTs w/ 1 consumption, unless multiple signals received
	    continue;
#endif
	} else
	    ++interesting;
	std::cout
	    << "  ERT/ID:  " << std::setw( 8 ) << r.first.first << "/" << std::setw( 2 ) << r.first.second
	    << std::endl;
	for ( auto &&c : r.second ) {
	    std::cout
		<< "    Consumption: " << std::setw( 6 ) << c.first
		<< std::endl;
	    for ( auto &&d : c.second ) {
		std::cout
		    << "      == " << d.first
		    << std::endl;
		for ( auto &&corr : d.second ) {
		    std::cout
			<< "         w/ " << corr.first << " corrections:  " << std::setw( 2 ) << corr.second << " x"
			<< std::endl;
		}
	    }
	}
    }

    std::cout
	<< "Detected " << assert.failures << " failures"
	<< ", " << interesting << " interesting ERT readings"
	<< std::endl;
    return ( assert.failures || ! interesting )? 1 : 0;
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
