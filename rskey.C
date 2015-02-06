

#include <ezpwd/rs>
#include <ezpwd/serialize>
#include <ezpwd/corrector>

#include <ezpwd/definitions>

#include "rskey.h"		// C API declarations

// 
// Reed-Solomon corrected data encoding in the specified number of data/parity symbols
// 
//     Base-32 encodes a block of data, with Reed-Solomon parity.  Each base-32
// digit encodes 32 values (5 bits).  Therefore, 64 bits of data payload requires
// 64/5 == ~13 data symbols.
// 
//                                   capacity
//     data----------  parity  total errors  erase.  eg.
//     bits    base32       
//     32       7      3 40%   10    1       3       ABCDE-FGH1K
//     64      13      2 15%   15    1       2       ABCDE-FGH1K-LMN0P
//     64      13      7 50%   20    3       7       ABCDE-FGH1K-LMN0P-01234
//     96      20      5 25%   25    2       5       ABCDE-FGH1K-LMN0P-01234-56789
// 
//     It is recommended that you put a record ID and a MAC (Message
// Authentication Code) in the data payload, and encrypt it.  On recovering an
// RSKEY code, decrypt the data payload to recover the ID and MAC.  Confirm the
// ID, by re-generating the MAC for that ID, and comparing with the MAC
// recovered from the decoded RSKEY.  If they match, the ID is valid.
// 
//     If you have a 32-bit ID (eg. a customer ID), and a 32-bit MAC (say, the
// HMAC-SHA1 of some immutable customer data, such as a record index, XOR-folded
// to 32 bits), you have a very sparse 64-bit space.  Even if your 32-bit (~4
// billion) ID space is completely full, the MAC will ensure that the
// probability of any one valid RSKEY guess coming up with a valid ID+MAC combo
// will be less then 1 in 4 billion.  This would constitute a quite strong
// validation of the legitimacy of the entered RSKEY.
// 

namespace ezpwd {
    template < size_t PARITY >
    int				rskey_encode( size_t, char *, size_t, size_t, size_t sep=0 );
    template < size_t PARITY >
    int				rskey_decode( size_t, char *, size_t, size_t );
} // namespace ezpwd

extern "C" {

    // ABCDE-FG
    int rskey_2_encode( size_t keysiz, char *buf, size_t buflen, size_t bufsiz, size_t sep )
    {
	return ezpwd::rskey_encode<2>( keysiz, buf, buflen, bufsiz, sep );
    }
    int rskey_2_decode( size_t keysiz, char *buf, size_t buflen, size_t bufsiz )
    {
	return ezpwd::rskey_decode<2>( keysiz, buf, buflen, bufsiz );
    }

    // ABCDE-FGH
    int rskey_3_encode( size_t keysiz, char *buf, size_t buflen, size_t bufsiz, size_t sep )
    {
	return ezpwd::rskey_encode<3>( keysiz, buf, buflen, bufsiz, sep );
    }
    int rskey_3_decode( size_t keysiz, char *buf, size_t buflen, size_t bufsiz )
    {
	return ezpwd::rskey_decode<3>( keysiz, buf, buflen, bufsiz );
    }

    // ABCDE-FGH1
    int rskey_4_encode( size_t keysiz, char *buf, size_t buflen, size_t bufsiz, size_t sep )
    {
	return ezpwd::rskey_encode<4>( keysiz, buf, buflen, bufsiz, sep );
    }
    int rskey_4_decode( size_t keysiz, char *buf, size_t buflen, size_t bufsiz )
    {
	return ezpwd::rskey_decode<4>( keysiz, buf, buflen, bufsiz );
    }

    // ABCDE-FGH1K
    int rskey_5_encode( size_t keysiz, char *buf, size_t buflen, size_t bufsiz, size_t sep )
    {
	return ezpwd::rskey_encode<5>( keysiz, buf, buflen, bufsiz, sep );
    }
    int rskey_5_decode( size_t keysiz, char *buf, size_t buflen, size_t bufsiz )
    {
	return ezpwd::rskey_decode<5>( keysiz, buf, buflen, bufsiz );
    }

} // extern "C"

namespace ezpwd {

template < size_t PARITY >					// number of R-S parity bytes
int				rskey_encode(
				    size_t		rawsiz,	// number of data payload bytes
				    char	       *buf,	// <= BITS bits raw data
				    size_t		buflen,	// data supplied (0-fill bytes to 'bits')
				    size_t		bufsiz,	// buffer available
				    size_t		sep )	// separator (eg. every 5 symbols)
{
    size_t			keysiz	= ezpwd::serialize::base32::encode_size( rawsiz ); // no padding
    int				res;
    try {
	if ( buflen > rawsiz )
	    throw std::runtime_error( 
	        std::string( "too much base-32 data (" ) << buflen << " > " << rawsiz << " bytes) provided" );
	std::string		key( keysiz, 0 );
#if defined( DEBUG )
	std::cout << "raw data: " << std::vector<uint8_t>( buf, buf + buflen ) << std::endl;
#endif
	ezpwd::serialize::base32::scatter( buf, buf + buflen, key.begin() );
#if defined( DEBUG )
	std::cout << "scatter:  " << std::vector<uint8_t>( key.begin(), key.end() ) << std::endl;
#endif
	ezpwd::corrector<PARITY,32>::encode( key );
#if defined( DEBUG )
	std::cout << "correct:  " << std::vector<uint8_t>( key.begin(), key.end() ) << std::endl;
#endif
	ezpwd::serialize::base32::encode( key.begin(), key.begin() + keysiz );
#if defined( DEBUG )
	std::cout << "base-32:  " << std::vector<uint8_t>( key.begin(), key.end() ) << std::endl;
#endif
	if ( key.size() > sep )
	    for ( size_t i = key.size() / sep - 1; i > 0; --i )
		key.insert( i * sep, 1, '-' );
#if defined( DEBUG )
	std::cout << "seperate: " << std::vector<uint8_t>( key.begin(), key.end() ) << std::endl;
#endif
	if ( key.size() + 1 > bufsiz  )
	    throw std::runtime_error(
	        std::string( "insufficient buffer provided for " ) << key.size() + 1 << " byte result" );
	std::copy( key.begin(), key.end(), buf );
	res				= key.size();
	buf[res]			= 0;
    } catch ( std::exception &exc ) {
	ezpwd::streambuf_to_buffer sbf( buf, bufsiz );
	std::ostream( &sbf )
	    << "rskey_encode<" << PARITY << "> failed: " << exc.what();
	res				= -1;
    }
    return res;
}

// 
// rskey_decode -- Recover BITS bits of data w/ PCT % parity, returning confidence
// 
//     If result is -'ve, then the decode has failed; the 'buf' will contain a
// NUL-terminated string describing the failure.
// 
//     Otherwise, BITS bits of data will be returned in 'buf', and the return
// value will be an integer percentage confidence, roughly the percentage of the
// parity that remained unconsumed by any required error correction.
// 
template < size_t PARITY >
int				rskey_decode(
				    size_t		rawsiz,	// number of data payload bytes
				    char	       *buf,
				    size_t		buflen,	// buffer length used
				    size_t		bufsiz )// buffer available
{
    int				confidence;
    size_t			keysiz	= ezpwd::serialize::base32::encode_size( rawsiz );

    try {
	// 
	// Decode data (not parity) from base-32.  Every invalid symbol is considered an erasure.
	// Whitespace/'-' are ignored, and removed from decoded key.  No pad (-1) symbols allowed.
	// 
	//      0 0 0 G 4 - 0 Y Y Y U - X _ Q Y K - Y 1 2 0 G - T 8 P 8 4
	// --> 0000001004  001F1F1F1B  1E00181F13  1F01020010  1A08170804 w/1 erasures
	// 
	std::string		key( buf, buf + buflen );
#if defined( DEBUG )
	std::cout << "rskey:    " << std::vector<uint8_t>( key.begin(), key.end() ) << std::endl;
#endif
	std::vector<int>	erasures;
	ezpwd::serialize::base32::decode( key, &erasures, 0, serialize::ws_ignored, serialize::pd_invalid );
	if ( key.size() < rawsiz )
	    throw std::runtime_error( 
	        std::string( "too little base-32 data (" ) << buflen << " bytes) provided"
		<< "; need " << keysiz << " symbols for " << rawsiz << " bytes of decoded data" );

	// 
	// Re-encode supplied parity data back to base-32, if any (corrector expects all parity encoded)
	// 
	//     0000001004  001F1F1F1B  1E00181F13  1F01020010   1A08170804
	// --> 0000001004  001F1F1F1B  1E00181F13  1F01020010   T 8 P 8 4
	//                                                      ^^^^^^^^^^
#if defined( DEBUG )
	std::cout << "decoded:  " << std::vector<uint8_t>( key.begin(), key.end() ) << " w/" << erasures.size() << " erasures" << std::endl;
#endif
	if ( key.size() > keysiz ) {
	    ezpwd::serialize::base32::encode( key.begin() + keysiz, key.end() );
#if defined( DEBUG )
	    std::cout << "reencode: " << std::vector<uint8_t>( key.begin(), key.end() ) << std::endl;
#endif
	}

	//
	// Correct (or at least check) data payload using any available parity (min. DATSIZ data),
	// removing up to PARSIZ parity symbols if successful.  The result must be the expected
	// RAWSIZ.  If not, then the statistical correction was incorrect.  The minimum and maximum
	// resultant password size is the same!
	// 
	//     0000001004  001F1F1F1B  1E00181F13  1F01020010   T 8 P 8 4
	//     0000001004  001F1F1F1B  1E1F181F13  1F01020010 w/80% confidence
	//                               ^^
	confidence			= ezpwd::corrector<PARITY,32>::decode( key, erasures, keysiz, keysiz );
#if defined( DEBUG )
	std::cout << "correct:  " << std::vector<uint8_t>( key.begin(), key.end() ) 
		  << " w/" << confidence << "% confidence"
		  << std::endl;
#endif
	if ( confidence < 0 )
	    throw std::runtime_error(
	        std::string( "too many errors to recover original data; low confidence" ));
	if ( key.size() != keysiz ) // should not occur; correct only returns keys of correct size
	    throw std::runtime_error(
	        std::string( "too many errors to recover original data; incorrect size" ));

	// 
	// Gather and recover the original 8-bit data, from the (corrected) base-32 symbols.
	// 
	//     0000001004  001F1F1F1B  1E1F181F13  1F01020010 w/80% confidence
	// --> 00010203FFFEFDFC ~7F0881
	//
	try {
	    ezpwd::serialize::base32::gather( key.begin(), key.end(), buf );
	} catch ( std::exception &exc ) {
	    throw std::logic_error( "Key recovery invalid: '" << std::vector<uint8_t>( key.begin(), key.end() )
				    << "; " << exc.what() );
	}
#if defined( DEBUG )
	std::cout << "gathered: " << std::vector<uint8_t>( buf, buf + rawsiz ) << std::endl;
#endif
    } catch ( std::exception &exc ) {
	ezpwd::streambuf_to_buffer	sbf( buf, bufsiz );
	std::ostream( &sbf )
	    << "rskey_decode<" << PARITY << "> failed: " << exc.what();
	confidence			= -1;
    }
    return confidence;
}

} // namespace ezpwd
