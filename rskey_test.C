#include <vector>
#include <list>
#include <set>
#include <array>

#include <ezpwd/rs>
#include <ezpwd/corrector>
#include <ezpwd/serialize>
#include <ezpwd/asserter>

#include <ezpwd/definitions>	// must be included in one C++ compilation unit

#include "rskey.C"

// 
// Test RSKEY Reed-Solomon corrected data encoding in the specified number of data symbols
// 
typedef std::vector<uint8_t>	u8vec_t;
typedef std::vector<int8_t>	s8vec_t;


template <size_t PARITY>
void				test_rskey(
				    ezpwd::asserter    &assert,
				    const u8vec_t      &raw,
				    const std::string  &key,
				    size_t		sep = 5 )
{
    char			enc[1024];
    std::copy( raw.begin(), raw.end(), enc );
    int				encres	= ezpwd::rskey_encode<PARITY>( raw.size(), enc, raw.size(), sizeof enc, sep );
    if ( assert.ISEQUAL( key, std::string( enc )))
	std::cout << assert << std::endl;
    if ( assert.ISEQUAL( encres, int( key.size() )))
	std::cout << assert << std::endl;

    // Got the encoded key.  Increase the loss/error load 'til failure
    size_t			load;
    for ( load = 0; load < key.size() - ( sep ? key.size() / sep : 0 ); ++load ) {
	char			dec[1024];
	std::copy( key.begin(), key.end(), dec );
	// convert a "random" non-space non-erasure symbol to an erasure (or error, if >= 2 load)
	std::set<size_t>	pos; // error/erasure positions used
	for ( size_t e = 0, c = 0; e < load; ++c ) {
	    for ( size_t j = 0; j < key.size(); ++j ) {
		if ( pos.find( j ) == pos.end() and dec[j] != '-' ) {
		    if ((( c * 997 + j ) % 29 ) == 13 ) {
			if ( e+2 <= load ) {
			    // error.  Consumes 2 parity.  Flip low bit of decoded base-32 symbol and re-encode
			    e	       += 2;
			    dec[j]	= ezpwd::serialize::ezpwd<32>::encoder[
					      ezpwd::serialize::ezpwd<32>::decoder[
					          dec[j]] ^ 1];
			} else {
			    // erasure.  Consumes 1 parity
			    e	       += 1;
			    dec[j] = '_';
			}
			break;
		    }
		}
	    }
	}

	int			decres	= ezpwd::rskey_decode<PARITY>( raw.size(), dec, key.size(), sizeof dec );
	if ( assert.ISTRUE( decres >= 0 or load > PARITY )) {
		std::cout
		    << assert << ": Unexpected failure at " << load << "-symbol erasure load; should handle "
		    << PARITY << " symbols of erasure"
		    << std::endl;
	}
	if ( decres < 0 )
	    break;
    }
}

void				test_rskey_simple( ezpwd::asserter &assert )
{
    // 64 bits --> 13 base-32 symbols + 2 parity
    char			enc[1024] = "\x00\x01\x02\x03\xFF\xFE\xFD\xFC";

    int				encres	= rskey_2_encode( 8, enc, 8, sizeof enc, 5 );
    if ( assert.ISEQUAL( std::string( "000G4-0YYYU-XYQWE" ), std::string( enc )))
	std::cout << assert << std::endl;
    if ( assert.ISEQUAL( encres, 17 ))
	std::cout << assert << std::endl;

    char			dec[1024];
    int				decres;
    // Decode, no errors
    std::copy( enc, enc+encres+1, dec );
    decres				= rskey_2_decode( 8, dec, encres, sizeof dec );
    if ( decres < 0 )
	std::cout << dec << std::endl;
    if ( assert.ISEQUAL( std::string( R"""(\0010203FFFEFDFC)""" ), std::string() << u8vec_t( dec, dec+8 )))
	std::cout << assert << std::endl;
    if ( assert.ISEQUAL( decres, 100 ))
	std::cout << assert << std::endl;

    // Decode, 1 erasure
    std::copy( enc, enc+encres+1, dec );
    dec[1] = '_';
    decres				= rskey_2_decode( 8, dec, encres, sizeof dec );
    if ( decres < 0 )
	std::cout << dec << std::endl;
    if ( assert.ISEQUAL( std::string( R"""(\0010203FFFEFDFC)""" ), std::string() << u8vec_t( dec, dec+8 )))
	std::cout << assert << std::endl;
    if ( assert.ISEQUAL( decres, 50 ))
	std::cout << assert << std::endl;

    // 2 erasures (not yet entered)
    const char		       *mag	= "9MGNE-BHHCD-MVY00-00000-MVRFN";
    std::copy( mag, mag+strlen(mag), dec );
    decres				= rskey_5_decode( 12, dec, strlen( mag )-2, sizeof dec );
    if ( assert.ISEQUAL( decres, 60 ))
	std::cout << assert << std::endl;
    // one error == 2 erasures
    std::copy( mag, mag+strlen(mag), dec );
    dec[20]='X';
    decres				= rskey_5_decode( 12, dec, strlen( mag ), sizeof dec );
    if ( assert.ISEQUAL( decres, 60 ))
	std::cout << assert << std::endl;
    // use '_' to indicate erasures
    std::copy( mag, mag+strlen(mag), dec );
    dec[20]='_';
    dec[21]='_';
    decres				= rskey_5_decode( 12, dec, strlen( mag ), sizeof dec );
    if ( assert.ISEQUAL( decres, 60 ))
	std::cout << assert << std::endl;
}

// 
// base<N> codec tests
// 

void				test_hex( ezpwd::asserter &assert )
{
    std::string			original( "\xde\xad\xbe\xef" );
    std::string			deadbeef( "deadbeef" );
    std::string		        nibbles(  "\x0d\x0e\x0a\x0d\x0b\x0e\x0e\x0f" );

    ezpwd::serialize::base16::decode( deadbeef );
    if ( assert.ISEQUAL( std::string() << u8vec_t( deadbeef.begin(), deadbeef.end() ),
			 std::string() << u8vec_t( nibbles.begin(), nibbles.end() )))
	std::cout << assert << std::endl;
    ezpwd::serialize::base16::encode( deadbeef );
    if ( assert.ISEQUAL( deadbeef, std::string( "DEADBEEF" )))
	std::cout << assert << std::endl;

    u8vec_t			u8;
    ezpwd::serialize::base16::gather_standard( nibbles.begin(), nibbles.end(), std::back_insert_iterator<u8vec_t>( u8 ));
    if ( assert.ISEQUAL( original, std::string( u8.begin(), u8.end() )))
	std::cout << assert << std::endl;

    u8vec_t			u8_nibbles;
    ezpwd::serialize::base16::scatter_standard( u8.begin(), u8.end(), std::back_insert_iterator<u8vec_t>( u8_nibbles ));
    if ( assert.ISEQUAL( nibbles, std::string( u8_nibbles.begin(), u8_nibbles.end() )))
	std::cout << assert << std::endl;
}

typedef std::list<std::array<const std::string, 4>>
			       baseN_tests_t;

template < typename SERSTD, typename SEREZC >
void				test_baseN(
				    ezpwd::asserter    &assert,
				    const baseN_tests_t &tests )
{

    for ( auto &t : tests ) {
	std::string			e1( std::get<0>( t ));

	// scatter, into raw (unencoded) base-N data, with padding.  An instance of a
	// std::random_access_iterator, to force optimal algorithm (both with and without padding)
	u8vec_t				e1_vec_1;
	SEREZC::scatter_standard( e1.begin(), e1.end(), std::back_insert_iterator<u8vec_t>( e1_vec_1 ));
	u8vec_t				e1_vec_1_no_pad;
	SEREZC::scatter( e1.begin(), e1.end(), std::back_insert_iterator<u8vec_t>( e1_vec_1_no_pad ));
#if defined( DEBUG )
	std::cout << "scatter (rnd.): " << u8vec_t( e1.begin(), e1.end() ) << " --> " << e1_vec_1 << std::endl;
#endif
	if ( assert.ISEQUAL( e1_vec_1.size(), SEREZC::encode_size( e1.size(), ezpwd::serialize::pd_enforce )))
	    std::cout << assert << std::endl;
	if ( assert.ISEQUAL( e1_vec_1_no_pad.size(), SEREZC::encode_size( e1.size() )))
	    std::cout << assert << std::endl;
	if ( assert.ISTRUE( std::search( e1_vec_1.begin(),        e1_vec_1.end(),
					 e1_vec_1_no_pad.begin(), e1_vec_1_no_pad.begin() )
			    == e1_vec_1.begin() ))
	    std::cout << assert << "; failed to find no-pad scatter at front of padded scatter" << std::endl;

	// An instance of a std::forward_iterator, to force use of generic algorithm with more
	// iterator comparisons
	u8vec_t				e1_vec_2;
	{
	    std::istringstream		e1_iss( e1 );
	    std::istreambuf_iterator<char>
					e1_iss_beg( e1_iss ),
					e1_iss_end;
	    SEREZC::scatter( e1_iss_beg, e1_iss_end, std::back_insert_iterator<u8vec_t>( e1_vec_2 ),
					       ezpwd::serialize::pd_enforce );
	}
	if ( assert.ISEQUAL( e1_vec_1, e1_vec_2 ))
	    std::cout << assert << std::endl;	    

	if ( assert.ISEQUAL( std::string( std::get<1>( t )), std::string() << e1_vec_1 ))
	    std::cout << assert << std::endl;
	if ( assert.ISEQUAL( std::string( std::get<1>( t )), std::string() << e1_vec_2 ))
	    std::cout << assert << std::endl;

	// Now, convert the raw scattered 4/5/6-bit data to base-N RFC4648 Standard symbols, in-place
	std::string		e1_s( e1_vec_1.begin(), e1_vec_1.end() );
	SERSTD::encode( e1_s.begin(), e1_s.end() );
#if defined( DEBUG )
	std::cout << "baseN::encode (standard): " << e1_vec_1  << " --> " << e1_s << std::endl;
#endif
	if ( assert.ISEQUAL( e1_s, std::get<2>( t )))
	    std::cout << assert << std::endl;

	// Now get rid of the pad symbols for the baseN ezpwd encoding test
	std::string		e1_e( e1_vec_1.begin(), e1_vec_1.end() );
	while ( e1_e.size() and e1_e.back() == EOF )
	    e1_e.pop_back();
	SEREZC::encode( e1_e.begin(), e1_e.end() );
#if defined( DEBUG )
	std::cout << "baseN::encode (ezpwd):    " << e1_vec_1  << " --> " << e1_e << std::endl;
#endif
	if ( assert.ISEQUAL( e1_e, std::get<3>( t )))
	    std::cout << assert << std::endl;
	
	// Decode back to the original 4/5/6-bit binary data in-place, from both the standard (with
	// pad) and ezcod (not padded) base-N encoded data.

	// _s -- Standard (with padding)
	std::string		d1_s( e1_s );
	SERSTD::decode_standard( d1_s );
	if ( assert.ISEQUAL( std::get<1>( t ), std::string() << u8vec_t( d1_s.begin(), d1_s.end() )))
	    std::cout << assert << std::endl;

	// _e -- Ezcod (without padding)
	std::string		d1_e( e1_e );
	SEREZC::decode( d1_e ); // default is ws_ignored, pd_ignored
	// It will match the decoding, but not include the trailing padding (FF) chars
	if ( assert.ISTRUE( std::get<1>( t ).find( std::string() << u8vec_t( d1_e.begin(), d1_e.end() )) == 0 ))
	    std::cout << assert << std::endl;

	// Finally, gather up the 4/5/6-bit chunks back into the original 8-bit data, using pd_ignored
	// (no padding, the default) first, and then pd_enforce.  Make sure that the predicted
	// baseN::encode_size is correct.

	u8vec_t			d1_vec_s; // pd_enforce
	SEREZC::gather_standard( d1_s.begin(), d1_s.end(), std::back_insert_iterator<u8vec_t>( d1_vec_s ));
	if ( assert.ISEQUAL( std::get<0>( t ), std::string( d1_vec_s.begin(), d1_vec_s.end() )))
	    std::cout << assert << std::endl;
	if ( assert.ISEQUAL( d1_s.size(), SEREZC::encode_size( std::get<0>( t ).size(), ezpwd::serialize::pd_enforce )))
	    std::cout << assert << "; predicted " << std::get<0>( t ) << " --> " << u8vec_t( d1_s.begin(), d1_s.end() ) <<
		" should be " << SEREZC::encode_size( std::get<0>( t ).size(), ezpwd::serialize::pd_enforce  ) << " bytes " << std::endl;

	u8vec_t			d1_vec_e;  // pd_invalid: default
	SEREZC::gather( d1_e.begin(), d1_e.end(), std::back_insert_iterator<u8vec_t>( d1_vec_e ));
	if ( assert.ISEQUAL( std::get<0>( t ), std::string( d1_vec_e.begin(), d1_vec_e.end() )))
	    std::cout << assert << std::endl;
	if ( assert.ISEQUAL( d1_e.size(), SEREZC::encode_size( std::get<0>( t ).size() ))) // default: pd_ignored
	    std::cout << assert << "; predicted " << std::get<0>( t ) << " --> " <<  u8vec_t( d1_e.begin(), d1_e.end() ) <<
		" should be " << SEREZC::encode_size( std::get<0>( t ).size() ) << " bytes " << std::endl;
    }
}

void				test_base16( ezpwd::asserter &assert )
{
    test_baseN< ezpwd::serialize::base16_standard, ezpwd::serialize::base16 >( assert,
        {
	    // original scatter to 4-bit chunks				base16_standard		base16
	    { "a",	R"""(0601)""",					"61",			"61" },
	    { "ab",	R"""(06010602)""",				"6162",			"6162" },
	    { "\xff\x01\x80",
			R"""(0F0F\001\b\0)""",				"FF0180",		"FF0180" },
	    { std::string( 1, '\x00' ) + "\x01",
			R"""(\0\0\001)""",				"0001",			"0001" },
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

          R"""(\0\0\001\002\003\004\005\006\0\a\0\b\0\t\0\n\0\v\0\f\0\r\00E\00F)"""
          R"""(01\001010102010301040105010601\a01\b01\t01\n01\v01\f01\r010E010F)"""
          R"""(02\002010202020302040205020606\002\b02\t02\n02\v02\f02\r020E020F)"""
          R"""(03\003010302030303040305030603\a03\b03\t03\n03\v03\f03\r030E030F)"""
          R"""(04\004010402040304040405040604\a04\b04\t04\n04\v04\f04\r040E040F)"""
          R"""(05\005010502050305040505050605\a05\b05\t05\n05\v05\f05\r050E050F)"""
          R"""(06\006010602060306040605060606\a06\b06\t06\n06\v06\f06\r060E060F)"""
          R"""(\a\0\a01\a02\a03\a04\a05\a06\a\a\a\b\a\t\a\n\a\v\a\f\a\r\a0E\a0F)"""
          R"""(\b\0\b01\b02\b03\b04\b05\b06\b\a\b\b\b\t\b\n\b\v\b\f\b\r\b0E\b0F)"""
          R"""(\t\0\t01\t02\t03\t04\t05\t06\t\a\t\b\t\t\t\n\t\v\t\f\t\r\t0E\t0F)"""
          R"""(\n\0\n01\n02\n03\n04\n05\n06\n\a\n\b\n\t\n\n\n\v\n\f\n\r\n0E\n0F)"""
          R"""(\v\0\v01\v02\v03\v04\v05\v06\v\a\v\b\v\t\v\n\v\v\v\f\v\r\v0E\v0F)"""
          R"""(\f\0\f01\f02\f03\f04\f05\f06\f\a\f\b\f\t\f\n\f\v\f\f\f\r\f0E\f0F)"""
          R"""(\r\0\r01\r02\r03\r04\r05\r06\r\a\r\b\r\t\r\n\r\v\r\f\r\r\r0E\r0F)"""
          R"""(0E\00E010E020E030E040E050E060E\a0E\b0E\t0E\n0E\v0E\f0E\r0E0E0E0F)"""
          R"""(0F\00F010F020F030F040F050F060F\a0F\b0F\t0F\n0F\v0F\f0F\r0F0E0F0F)""",
		
          R"""(000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F)"""
          R"""(202122232425266028292A2B2C2D2E2F303132333435363738393A3B3C3D3E3F)"""
          R"""(404142434445464748494A4B4C4D4E4F505152535455565758595A5B5C5D5E5F)"""
          R"""(606162636465666768696A6B6C6D6E6F707172737475767778797A7B7C7D7E7F)"""
          R"""(808182838485868788898A8B8C8D8E8F909192939495969798999A9B9C9D9E9F)"""
          R"""(A0A1A2A3A4A5A6A7A8A9AAABACADAEAFB0B1B2B3B4B5B6B7B8B9BABBBCBDBEBF)"""
          R"""(C0C1C2C3C4C5C6C7C8C9CACBCCCDCECFD0D1D2D3D4D5D6D7D8D9DADBDCDDDEDF)"""
          R"""(E0E1E2E3E4E5E6E7E8E9EAEBECEDEEEFF0F1F2F3F4F5F6F7F8F9FAFBFCFDFEFF)""",

          R"""(000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F)"""
          R"""(202122232425266028292A2B2C2D2E2F303132333435363738393A3B3C3D3E3F)"""
          R"""(404142434445464748494A4B4C4D4E4F505152535455565758595A5B5C5D5E5F)"""
          R"""(606162636465666768696A6B6C6D6E6F707172737475767778797A7B7C7D7E7F)"""
          R"""(808182838485868788898A8B8C8D8E8F909192939495969798999A9B9C9D9E9F)"""
          R"""(A0A1A2A3A4A5A6A7A8A9AAABACADAEAFB0B1B2B3B4B5B6B7B8B9BABBBCBDBEBF)"""
          R"""(C0C1C2C3C4C5C6C7C8C9CACBCCCDCECFD0D1D2D3D4D5D6D7D8D9DADBDCDDDEDF)"""
          R"""(E0E1E2E3E4E5E6E7E8E9EAEBECEDEEEFF0F1F2F3F4F5F6F7F8F9FAFBFCFDFEFF)""",

          },
	} );
}

void				test_base32( ezpwd::asserter &assert )
{
    test_baseN< ezpwd::serialize::base32_standard, ezpwd::serialize::base32 >( assert,
        {
	    // original scatter to 5-bit chunks				standard		ezcod
	    { "a",	R"""(\f04FFFFFFFFFFFF)""",			"ME======",		"C4" },
	    { "ab",	R"""(\f0511\0FFFFFFFF)""",			"MFRA====",		"C5H0" },
	    { "abc",	R"""(\f05110606FFFFFF)""",			"MFRGG===",		"C5H66" },
	    { "abcd",	R"""(\f0511060619\0FF)""",			"MFRGGZA=",		"C5H66R0" },
	    { "abcde",	R"""(\f05110606190305)""",			"MFRGGZDF",		"C5H66R35" },
	    { "abcde0",	R"""(\f0511060619030506\0FFFFFFFFFFFF)""",	"MFRGGZDFGA======",	"C5H66R3560" },
	    // RFC4648 test vectors
	    { "foo",	R"""(\f1917161EFFFFFF)""",			"MZXW6===",		"CRPNX" },
	    { "foobar",	R"""(\f1917161E1813010E\bFFFFFFFFFFFF)""",	"MZXW6YTBOI======",	"CRPNXQK1E8" },
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

          R"""(\0\0\01004\01804\01403\00E02\0\t01\b05101803\b0E011C\b0102041013)"""
          R"""(0210\n11\f0518180304\r0116\a\01D03180F12\0\b\t0204\f1202\n\t13\0)"""
          R"""(05\0141214\n19\f051417021E\f011106\b1913\b\r\t16061C1C03120E11\e)"""
          R"""(\a101E131C0F1A\0\b05010406110205\b1903141012\n\n\t\r06041A13120F)"""
          R"""(\n01\b1504141A14\n15\v050E160219\v\t\r151817\n1E\v1D100602181303)"""
          R"""(\f111216\f19\e\b\r05150616\e03\r\r191717\01C\v120E\r1A\a\n1D1317)"""
          R"""(0F011C17141E\e1C0F151F\a1F\0040110\n0118\t01\f06101E04\b130214\v)"""
          R"""(111206181D031C101206\t\t\a050415121A\v191106\f1A130E0E\t\e\a141F)"""
          R"""(1402101A05\b1D04141613\n0F\n05\t15\n151A19\v\r0E151E18\v03\f1513)"""
          R"""(16121A\e\r\r1D1817061D\v170F051D171A1F1C01100E02180F02\f\v1116\a)"""
          R"""(1903041C15121E\f1917\a\f1F1406111A\v\t1D\t150E161A1F\f\r131616\e)"""
          R"""(\e130E1D1D171F\01C\a110E\a19\a051C\e131E111A0F\n1D0F160E\e\e170F)"""
          R"""(1E03181F051C1F141E17\e0F0F1E\a191F\v1D1F191F0F1E1F1CFFFFFFFFFFFF)""",

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
	} );
}

void				test_base64( ezpwd::asserter &assert )
{
    test_baseN< ezpwd::serialize::base64_standard, ezpwd::serialize::base64 >( assert,
        {
	    // original scatter to 6-bit chunks				standard		ezcod
	    { "",       R"""()""",					"",			"" },
	    { "a",	R"""(1810FFFF)""",				"YQ==",			"YQ" },
	    { "ab",	R"""(1816\bFF)""",				"YWI=",			"YWI" },
	    { "f",	R"""(19  FFFF)""",				"Zg==", 		"Zg" },
	    { "fo", 	R"""(19 & <FF)""", 				"Zm8=",			"Zm8" },
	    { "foo",	R"""(19 & = /)""",				"Zm9v",			"Zm9v" },
	    { "foob",	R"""(19 & = /18  FFFF)""",			"Zm9vYg==",		"Zm9vYg" },
	    { "fooba",	R"""(19 & = /18 &04FF)""",			"Zm9vYmE=",		"Zm9vYmE" },
	    { "foobar",	R"""(19 & = /18 &05 2)""",			"Zm9vYmFy",		"Zm9vYmFy" },
	    { "\x14\xfb\x9c\x03\xd9\x7e",
		        R"""(050F .1C\0 = % >)""",			"FPucA9l+",		"FPucA9l+" },
	    { "\xff\xff\xff\xff\xff\xff",
			R"""( ? ? ? ? ? ? ? ?)""",			"////////",		"........" },
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

          R"""(\0\00402\0 0100501  1C\b0210 (\v03\0 40E03 1\01104 !\f1405111817)"""
          R"""(0601 $1A06 1 01D\a ! <  \b12\b #\t0214 &1802   )\n\" , ,\v12 8 /)"""
          R"""(\f0304 2\f 310 5\r #1C 80E13 ( ;0F03 4 >0F 4010110 $\r04111419\a)"""
          R"""(1204 %\n12 4 1\r13 $ =101415\t131505151615 5 !1916 % -1C1715 91F)"""
          R"""(180605\"18 611 %19 &1D (1A16 ) +\e06 5 .\e 701 11C\'\r 41D1719 7)"""
          R"""(1E\a % :1E 7 1 =1F\' >\0  18\n03 !\b1606 ! 8\"\t\" ( .\f #18 :0F)"""
          R"""( $\t0612 $ 91215 % )1E18 &19 *\e\'\t 61E\' :02 ! ( *0E $ )1A1A\')"""
          R"""( *\n & * * : 2 - + * > 0 ,\e\n 3 -\v16 6 - ;\" 9 . + . < /\e : ?)"""
          R"""( 0\f\a02 0 <1305 1 ,1F\b 21C +\v 3\f 70E 3 =0311 4 -0F14 51D\e17)"""
          R"""( 6\r\'1A 6 = 31D 7 - ?   81E\v # 90E17 & 9 > # ) : . / , ;1E ; /)"""
          R"""( <0F\a 2 < ?13 5 = /1F 8 >1F + ; ?0F 7 > ? 0FFFF)""",
		
	      "AAECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8gISIjJCUmYCgpKissLS4v"
	      "MDEyMzQ1Njc4OTo7PD0+P0BBQkNERUZHSElKS0xNTk9QUVJTVFVWV1hZWltcXV5f"
	      "YGFiY2RlZmdoaWprbG1ub3BxcnN0dXZ3eHl6e3x9fn+AgYKDhIWGh4iJiouMjY6P"
	      "kJGSk5SVlpeYmZqbnJ2en6ChoqOkpaanqKmqq6ytrq+wsbKztLW2t7i5uru8vb6/"
	      "wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm5+jp6uvs7e7v"
	      "8PHy8/T19vf4+fr7/P3+/w==",

	      "AAECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8gISIjJCUmYCgpKissLS4v"
	      "MDEyMzQ1Njc4OTo7PD0+P0BBQkNERUZHSElKS0xNTk9QUVJTVFVWV1hZWltcXV5f"
	      "YGFiY2RlZmdoaWprbG1ub3BxcnN0dXZ3eHl6e3x9fn+AgYKDhIWGh4iJiouMjY6P"
	      "kJGSk5SVlpeYmZqbnJ2en6ChoqOkpaanqKmqq6ytrq+wsbKztLW2t7i5uru8vb6."
	      "wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t.g4eLj5OXm5+jp6uvs7e7v"
	      "8PHy8.T19vf4+fr7.P3+.w" },
	} );
}

int				main( int, char ** )
{
    std::cout
	<< "rskey tests ..."
	<< std::endl;

    ezpwd::asserter		assert;
    
    test_base32( assert );
    test_base64( assert );
    test_hex( assert );
    test_base16( assert );
    test_rskey_simple( assert );
    // 8 bytes --> 13 base-32 symbols + 2 parity
    test_rskey<2>( assert,
        u8vec_t { 0x00, 0x01, 0x02, 0x03, 0xFF, 0xFE, 0xFD, 0xFC }, 
        "000G4-0YYYU-XYQWE" );
    // 12 bytes --> 20 base-32 symbols + 5 parity
    test_rskey<5>( assert,
        u8vec_t { 0x00, 0x01, 0x02, 0x03, 0xFF, 0xFE, 0xFD, 0xFC, 0x7e, 0x7f, 0x08, 0x81 },
        "000G4-0YYYU-XYQYK-Y120G-T8P84" );

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
