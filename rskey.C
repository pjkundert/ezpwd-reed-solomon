

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
//     32       7      3       10    1       3       ABCDE-FGH1K
//     64      13      2       15    1       2       ABCDE-FGH1K-LMN0P
//     64      13      7       20    3       7       ABCDE-FGH1K-LMN0P-01234
//     96      20      5       25    2       5       ABCDE-FGH1K-LMN0P-01234-56789
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
template < size_t BITS=64, size_t PCT=10 >
int				rskey_encode(
				    char	       *buf,	// <= BITS bits raw data
				    size_t		len,	// data supplied (0-fill to B bits)
				    size_t		siz,	// buffer available
				    size_t		sep = 5 )// separator every 5 bytes
{
#if defined( DEBUG ) && DEBUG >= 1
    std::cout
	<< "rskey_encode<BITS=" << BITS << ",PCT=" << PCT << ">("
	<< ", enc[" << len << "] == " << std::vector<uint8_t>( (uint8_t*)buf, (uint8_t*)buf + len )
	<< " ) --> ";
#endif
    // Compute data, parity and total key sizes, in base-32 encoded bytes
    constexpr bool		no_pad	= false;
    constexpr size_t		RAWSIZ	= ( BITS + 7 ) / 8;
    constexpr size_t		DATSIZ	= ezpwd::serialize::base32::encode_size( RAWSIZ, no_pad );
    constexpr size_t		PARSIZ	= ( DATSIZ * PCT + 99 ) / 100;
  //constexpr size_t		KEYSIZ	= DATSIZ + PARSIZ;

    int				res;
    try {
	if ( len > RAWSIZ )
	    throw std::runtime_error( 
	        std::string( "too much base-32 data (" ) << len << " > " << RAWSIZ << " bytes / "
		<< BITS << " bits) provided" );
	std::string		key( DATSIZ, 0 );				// 00010203FFFEFDFC
	std::cout << "raw data: " << std::vector<uint8_t>( buf, buf+len ) << std::endl;
	ezpwd::serialize::base32::scatter( buf, buf+len, key.begin(), no_pad );	// 00010203FFFEFDFC
	std::cout << "scatter:  " << std::vector<uint8_t>( key.begin(), key.end() ) << std::endl;
	ezpwd::corrector<PARSIZ,32>::encode( key );
	std::cout << "correct:  " << std::vector<uint8_t>( key.begin(), key.end() ) << std::endl;
	ezpwd::serialize::base32::encode( key.begin(), key.begin() + DATSIZ );
	std::cout << "base-32:  " << std::vector<uint8_t>( key.begin(), key.end() ) << std::endl;
	if ( key.size() > sep )
	    for ( size_t i = key.size() / sep - 1; i > 0; --i )
		key.insert( i * sep, 1, '-' );
	std::cout << "seperate: " << std::vector<uint8_t>( key.begin(), key.end() ) << std::endl;
	if ( key.size() + 1 > siz  )
	    throw std::runtime_error(
	        std::string( "insufficient buffer provided for " ) << key.size() + 1 << " byte result" );
	std::copy( key.begin(), key.end(), buf );
	res				= key.size();
	buf[res]			= 0;
    } catch ( std::exception &exc ) {
	ezpwd::streambuf_to_buffer sbf( buf, siz );
	std::ostream( &sbf )
	    << "rskey_encode<BITS=" << BITS << ",PCT=" << PCT << "> failed: " << exc.what();
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
template < size_t BITS=64, size_t PCT=10 >
int				rskey_decode(
				    char	       *buf,
				    size_t		len,	// buffer length used
				    size_t		siz )	// buffer available
{
    int				confidence;

    // Compute data, parity and total key sizes, in base-32 encoded bytes
    constexpr bool		no_pad	= false;
    constexpr size_t		RAWSIZ	= ( BITS + 7 ) / 8;
    constexpr size_t		DATSIZ	= ezpwd::serialize::base32::encode_size( RAWSIZ, no_pad );
    constexpr size_t		PARSIZ	= ( DATSIZ * PCT + 99 ) / 100;
  //constexpr size_t		KEYSIZ	= DATSIZ + PARSIZ;

    try {
	if ( len < DATSIZ )
	    throw std::runtime_error( 
	        std::string( "too little base-32 data (" ) << len << " bytes) provided"
		<< "; need " << DATSIZ << " bytes for " << RAWSIZ << " bytes / " << BITS
		<< " bits of decoded data" );
		
	// Convert data (not parity) from base-32.  Every invalid symbol is considered an erasure.
	std::string		key( buf, buf+len );
	std::cout << "rskey:    " << std::vector<uint8_t>( key.begin(), key.end() ) << std::endl;
	std::vector<int>	erasures;
	ezpwd::serialize::base32::decode( key, &erasures, 0,
	    ezpwd::serialize::ws_ignore, ezpwd::serialize::pd_invalid );
	std::cout << "decoded:  " << std::vector<uint8_t>( key.begin(), key.end() ) << " w/" << erasures.size() << " erasures" << std::endl;
	if ( key.size() > DATSIZ ) {
	    // There is some parity; re-encode it to base-32 (corrector expects all parity encoded)
	    ezpwd::serialize::base32::encode( key.begin() + DATSIZ, key.end() );
	    std::cout << "reencode: " << std::vector<uint8_t>( key.begin(), key.end() ) << std::endl;
	}
	
	// Correct (or at least check) data payload using any available parity (min. DATSIZ data),
	// removing parity symbols if successful.
	confidence			= ezpwd::corrector<PARSIZ,32>::decode( key, DATSIZ, erasures );
	if ( confidence < 0 )
	    throw std::runtime_error(
	        std::string( "too many errors to recover original data" ));
	// Gather and recover the original 8-bit data.
	ezpwd::serialize::base32::gather( key.begin(), key.end(), buf, true );
	std::cout << "gathered: " << std::vector<uint8_t>( buf, buf + RAWSIZ ) << std::endl;
    } catch ( std::exception &exc ) {
	ezpwd::streambuf_to_buffer	sbf( buf, siz );
	std::ostream( &sbf )
	    << "rskey_decode<BITS=" << BITS << ",PCT=" << PCT << "> failed: " << exc.what();
	confidence			= -1;
    }
    return confidence;
}

extern "C" {

    int rskey_64_10_encode( char *buf, size_t len, size_t siz )
    {
	return rskey_encode<64,10>( buf, len, siz );
    }
    int rskey_64_10_decode( char *buf, size_t len, size_t siz )
    {
	return rskey_decode<64,10>( buf, len, siz );
    }

} // extern "C"
