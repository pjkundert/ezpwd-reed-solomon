#include <vector>
#include <list>
#include <tuple>

#include <ezpwd/rs>
#include <ezpwd/corrector>
#include <ezpwd/serialize>
#include <ezpwd/asserter>

// 
// Test RSKEY Reed-Solomon corrected data encoding in the specified number of data symbols (default: 20)



int				main( int argc, char **argv )
{
    std::cout
	<< "rskey tests ..."
	<< std::endl;

    ezpwd::asserter		assert;

    for ( auto &t : std::list<std::tuple<std::string, std::string, std::string, std::string>> {
	    // original scatter to 5-bit chunks				standard		ezcod
	    { "a",	R"""(\f04FFFFFFFFFFFF)""",			"ME======",		"C4" },
	    { "ab",	R"""(\f051100FFFFFFFF)""",			"MFRA====",		"C5H0" },
	    { "abc",	R"""(\f05110606FFFFFF)""",			"MFRGG===",		"C5H66" },
	    { "abcd",	R"""(\f051106061900FF)""",			"MFRGGZA=",		"C5H66R0" },
	    { "abcde",	R"""(\f05110606190305)""",			"MFRGGZDF",		"C5H66R35" },
	    { "abcde0",	R"""(\f051106061903050600FFFFFFFFFFFF)""",	"MFRGGZDFGA======",	"C5H66R3560" },
	    // RFC4648 test vectors
	    { "foo",	R"""(\f1917161EFFFFFF)""",			"MZXW6===",		"CRPNX" },
	    { "foobar",	R"""(\f1917161E1813010E08FFFFFFFFFFFF)""",	"MZXW6YTBOI======",	"CRPNXQK1E8" },
	    { "\xff",	R"""(1F1CFFFFFFFFFFFF)""",			"74======",		"YV" },
	    { "\xff\xff",
			R"""(1F1F1F10FFFFFFFF)""",			"777Q====",		"YYYG" },
	    { "\xff\xff\xff",
			R"""(1F1F1F1F1EFFFFFF)""",			"77776===",		"YYYYX" },
	    { "\xff\xff\xff\xff",
			R"""(1F1F1F1F1F1F18FF)""",			"777777Y=",		"YYYYYYQ" },
	    { "\xff\xff\xff\xff\xff",
			R"""(1F1F1F1F1F1F1F1F)""",			"77777777",		"YYYYYYYY" },
	    { "\xff\xff\xff\xff\xff\xff",
			R"""(1F1F1F1F1F1F1F1F1F1CFFFFFFFFFFFF)""",	"7777777774======",	"YYYYYYYYYV" },
	    { std::string( 1, '\x00' ) + "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
	      "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f"
	      " !\"#$%&`()*+,-./"
	      "0123456789:;<=>?"
	      "@ABCDEFGHIJKLMNO"
	      "PQRSTUVWXYZ[\\]^_"
	      "`abcdefghijklmno"
	      "pqrstuvwxyz{|}~\x7f"
	      "\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f"
	      "\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f"
	      "\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad\xae\xaf"
	      "\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf"
	      "\xc0\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce\xcf"
	      "\xd0\xd1\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf"
	      "\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef"
	      "\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff",

	  R"""(0000001004001804001403000E0200\t010805101803080E011C080102041013)"""
	  R"""(0210\n11\f0518180304\r011607001D03180F120008\t0204\f1202\n\t1300)"""
	  R"""(0500141214\n19\f051417021E\f01110608191308\r\t16061C1C03120E111B)"""
	  R"""(07101E131C0F1A000805010406110205081903141012\n\n\t\r06041A13120F)"""
	  R"""(\n01081504141A14\n15\v050E160219\v\t\r151817\n1E\v1D100602181303)"""
	  R"""(\f111216\f191B08\r051506161B03\r\r191717001C\v120E\r1A07\n1D1317)"""
	  R"""(0F011C17141E1B1C0F151F071F00040110\n0118\t01\f06101E0408130214\v)"""
	  R"""(111206181D031C101206\t\t07050415121A\v191106\f1A130E0E\t1B07141F)"""
          R"""(1402101A05081D04141613\n0F\n05\t15\n151A19\v\r0E151E18\v03\f1513)"""
          R"""(16121A1B\r\r1D1817061D\v170F051D171A1F1C01100E02180F02\f\v111607)"""
          R"""(1903041C15121E\f191707\f1F1406111A\v\t1D\t150E161A1F\f\r1316161B)"""
          R"""(1B130E1D1D171F001C07110E071907051C1B131E111A0F\n1D0F160E1B1B170F)"""
          R"""(1E03181F051C1F141E171B0F0F1E07191F\v1D1F191F0F1E1F1CFFFFFFFFFFFF)""",

	      "AAAQEAYEAUDAOCAJBIFQYDIOB4IBCEQTCQKRMFYYDENBWHA5DYPSAIJCEMSCKJTA"
	      "FAUSUKZMFUXC6MBRGIZTINJWG44DSOR3HQ6T4P2AIFBEGRCFIZDUQSKKJNGE2TSP"
	      "KBIVEU2UKVLFOWCZLJNVYXK6L5QGCYTDMRSWMZ3INFVGW3DNNZXXA4LSON2HK5TX"
	      "PB4XU634PV7H7AEBQKBYJBMGQ6EITCULRSGY5D4QSGJJHFEVS2LZRGM2TOOJ3HU7"
	      "UCQ2FI5EUWTKPKFJVKV2ZLNOV6YLDMVTWS23NN5YXG5LXPF5X274BQOCYPCMLRWH"
	      "ZDE4VS6MZXHM7UGR2LJ5JVOW27MNTWW33TO55X7A4HROHZHF43T6R2PK5PWO33XP"
	      "6DY7F47U6X3PP6HZ7L57Z7P674======",
	      "000G40Q40L30E209185GQ38E1V8124GK2GAHC5QQ34D1N70W3QFJ08924CJ2A9K0"
	      "50LJLARC5LP2XC1H68RK8D9N6VV3JEHU7GXKVFT085146H258R3LGJAA9D64TKJF"
	      "A18M4LTLAMB5EN2RB9DMQPAXBWG62QK3CHJNCRU8D5M6NU3DDRPP0VBJEDT7AWKP"
	      "F1VPLXUVFMY7Y041GA1Q91C6GX48K2LBHJ6QW3VGJ699754MJTBRH6CTKEE9U7LY"
	      "L2GT58W4LNKAFA59MAMTRBDEMXQB3CMKNJTUDDWQP6WBPF5WPTYV1GE2QF2CBHN7"
	      "R34VMJXCRP7CYL6HTB9W9MENTYCDKNNUUKEWWPY0V7HE7R75VUKXHTFAWFNEUUPF"
	      "X3QY5VYLXPUFFX7RYBWYRYFXYV" },
	} ) {

	std::string			e1( std::get<0>( t ));
	typedef std::vector<unsigned char>
	    u8vec_t;
	typedef std::vector<char>
	    s8vec_t;

	// An instance of a std::random_access_iterator, to force optimal algorithm
	u8vec_t				e1_vec_1;
	ezpwd::base32::scatter( e1.begin(), e1.end(), std::back_insert_iterator<u8vec_t>( e1_vec_1 ), true );
#if defined( DEBUG )
	std::cout << "scatter (rnd.): " << u8vec_t( e1.begin(), e1.end() ) << " --> " << e1_vec_1 << std::endl;
#endif
	// An instance of a std::forward_iterator, to force use of algorithm with more iterator
	// comparisons
	u8vec_t				e1_vec_2;
	{
	    std::istringstream		e1_iss( e1 );
	    std::istreambuf_iterator<char>	e1_iss_beg( e1_iss );
	    std::istreambuf_iterator<char>	e1_iss_end;
	    ezpwd::base32::scatter( e1_iss_beg, e1_iss_end, std::back_insert_iterator<u8vec_t>( e1_vec_2 ), true );
	}
#if defined( DEBUG )
	{
	    std::istringstream		e1_iss( e1 );
	    std::istreambuf_iterator<char>	e1_iss_beg( e1_iss );
	    std::istreambuf_iterator<char>	e1_iss_end;
	    std::cout << "scatter (fwd.): " <<  u8vec_t( e1_iss_beg, e1_iss_end )
		      << " --> " << e1_vec_2 << std::endl;
	}
#endif
	if ( assert.ISEQUAL( e1_vec_1, e1_vec_2 ))
	    std::cout << assert << std::endl;	    

	if ( assert.ISEQUAL( std::string( std::get<1>( t )), std::string() << e1_vec_1 ))
	    std::cout << assert << std::endl;
	if ( assert.ISEQUAL( std::string( std::get<1>( t )), std::string() << e1_vec_2 ))
	    std::cout << assert << std::endl;

	// Now, convert the raw scattered 5-bit data to base32, in-place
	std::string		e1_s( e1_vec_1.begin(), e1_vec_1.end() );
	ezpwd::base32::encode( e1_s.begin(), e1_s.end(), '=', ezpwd::base32::encoder_standard );
#if defined( DEBUG )
	std::cout << "base32::encode (standard): " << e1_vec_1  << " --> " << e1_s << std::endl;
#endif
	if ( assert.ISEQUAL( e1_s, std::get<2>( t )))
	    std::cout << assert << std::endl;

	// Now get rid of the pad symbols for the base32 ezpwd encoding test
	std::string		e1_e( e1_vec_1.begin(), e1_vec_1.end() );
	while ( e1_e.size() and e1_e.back() == EOF )
	    e1_e.pop_back();
	ezpwd::base32::encode( e1_e.begin(), e1_e.end() );
#if defined( DEBUG )
	std::cout << "base32::encode (ezcod):    " << e1_vec_1  << " --> " << e1_e << std::endl;
#endif
	if ( assert.ISEQUAL( e1_e, std::get<3>( t )))
	    std::cout << assert << std::endl;
	
	// Now, decode back to the original 5-bit binary data in-place, from both the
	// standard (with pad) and ezcod (not padded) base32 encoded data.
	std::string		d1_s( e1_s );
	ezpwd::base32::decode( d1_s, 0, 0, ezpwd::base32::ws_ignore, ezpwd::base32::pd_keep, ezpwd::base32::decoder_standard );
	if ( assert.ISEQUAL( std::get<1>( t ), std::string() << u8vec_t( d1_s.begin(), d1_s.end() )))
	    std::cout << assert << std::endl;

	std::string		d1_e( e1_e );
	ezpwd::base32::decode( d1_e );
	// It will match the decoding, but not including the trailing padding (FF) chars
	if ( assert.ISTRUE( std::get<1>( t ).find( std::string() << u8vec_t( d1_e.begin(), d1_e.end() )) == 0 ))
	    std::cout << assert << std::endl;

	// Finally, gather up the 5-bit chunks back into the original 8-bit data.  With standard
	// encoding, this should always be identical.  With ezcod encoding (which doesn't emit pad
	// symbols by default), if there should have been padding, then there may be extra symbols
	// emitted.  This is expected; 
	u8vec_t			d1_vec_s;
	ezpwd::base32::gather( d1_s.begin(), d1_s.end(), std::back_insert_iterator<u8vec_t>( d1_vec_s ));
	if ( assert.ISEQUAL( std::get<0>( t ), std::string( d1_vec_s.begin(), d1_vec_s.end() )))
	    std::cout << assert << std::endl;

	u8vec_t			d1_vec_e;
	ezpwd::base32::gather( d1_e.begin(), d1_e.end(), std::back_insert_iterator<u8vec_t>( d1_vec_e ), true );
	if ( assert.ISEQUAL( std::get<0>( t ), std::string( d1_vec_e.begin(), d1_vec_e.end() )))
	    std::cout << assert << std::endl;
    }

    if ( assert.failures )
	std::cout
	    << __FILE__ << " fails " << assert.failures << " tests"
	    << std::endl;
    else
	std::cout
	    << "  ...all tests passed."
	    << std::endl;

    return assert.failures ? 1 : 0;
}
