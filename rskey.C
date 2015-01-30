

#include <ezpwd/rs>

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
template < size_t B=64, size_t P=10 > // B=<bits>, P=<percent>
int				rskey_encode(
				    unsigned char      *enc,
				    size_t		siz,		// data supplied
				    size_t		maximum )	// maximum
									// space availalbe
{
#if defined( DEBUG ) && DEBUG >= 1
    std::cout
	<< "rskey_encode<" << B << "," << P << ">("
	<< ", buf[" << siz << "] == " << std::vector<uint8_t>( (uint8_t*)enc, (uint8_t*)enc + siz )
	<< std::endl;
#endif
    static const size_t		DATA	= ((B+4)/5);
    static const size_t		PARI	= (DATA*P+99)/100;
    static const size_t		SIZE	= DATA+PARI;

    ezpwd::corrector<SIZE>	correct;

    int				res;
    try {
	const std::string      &str( ezpwd::ezcod<P,L>( lat, lon ).encode() );
	if ( str.size() + 1 > siz )
	    throw std::runtime_error( "insufficient buffer provided" );
	std::copy( str.begin(), str.end(), enc );
	res				= str.size();
	enc[res]			= 0;
    } catch ( std::exception &exc ) {
	ezpwd::streambuf_to_buffer sbf( enc, siz );
	std::ostream( &sbf )
	    << "ezcod_3_encode<" << P << "," << L << ">(" << lat << ", " << lon << ") failed: "
	    << exc.what();
	res				= -1;
    }
    return res;
}

// 
// Decode lat/lon position in degrees and (optionally) accuracy in m, returning confidence in %.
// 

template < size_t P=1, size_t L=9 >
int				rskey_decode(
				    char	       *buf,
				    size_t		siz,
				    double	       *lat	= 0,
				    double	       *lon	= 0,
				    double	       *acc	= 0 )
{
    int				confidence;
    std::string			location( buf );
    ezpwd::ezcod<P,L>		decoder( location );
    try {
	confidence			= decoder.confidence;
#if defined( DEBUG ) && DEBUG > 1
	std::cout
	    << "ezcod_3_decode<" << P << "," << L << ">(\"" << location
	    << "\") == " << confidence << "% confidence: "
	    << " lat (" << (void *)lat << ") == " << decoder.latitude
	    << ", lon (" << (void *)lon << ") == " << decoder.longitude
	    << ", acc (" << (void *)acc << ") == " << decoder.accuracy
	    << std::endl;
#endif
	if ( lat )
	    *lat			= decoder.latitude;
	if ( lon )
	    *lon			= decoder.longitude;
	if ( acc )
	    *acc			= decoder.accuracy;
    } catch ( std::exception &exc ) {
	ezpwd::streambuf_to_buffer	sbf( buf, siz );
	std::ostream( &sbf )
	    << "ezcod_3_decode<" << P << "," << L << ">(\"" << location << "\") failed: "
	    << exc.what();
	confidence			= -1;
    }
    return confidence;
}

extern "C" {

    int rskey_64_10_encode( char *enc, size_t siz )
    {
	return ezcod_3_encode<64,10>( enc, siz );
    }
    int rskey_64_10_decode( char *dec, size_t siz, size_t min )
    {
	return rskey_decode<64,10>( dec, siz, min );
    }

} // extern "C"
