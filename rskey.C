

#include <ezpwd/rs>

#include "rskey.h"		// C API declarations

// 
// Reed-Solomon corrected data encoding in the specified number of data/parity symbols
// 
//     Base-32 encodes a block of data, with Reed-Solomon parity.  Each base-32
// digit encodes 32 values (5 bits).  Therefore, 64 bits of data payload requires
// 64/5 13 data symbols.  So, in 20 digits
// 
//                             capacity
//     data----------  parity  errors  erase.  eg.
//     bits    base32
//     32       7      3       1       3       ABCDE-FGH1K
//     64      13      2       1       2       ABCDE-FGH1K-LMN0P
//     64      13      7       3       7       ABCDE-FGH1K-LMN0P-01234
//     96      20      5       2       5       ABCDE-FGH1K-LMN0P-01234-56789
// 
// 
template < size_t B=64, size_t P=10 > // B=<bits>, P=<percent>
int				rskey_encode(
				    char      	       *enc,
				    size_t		siz )
{
#if defined( DEBUG ) && DEBUG >= 1
    std::cout
	<< "rskey_encode<" << B << "," << P << ">("
	<< ", buf[" << siz << "] == " << std::vector<uint8_t>( (uint8_t*)enc, (uint8_t*)enc + siz )
	<< std::endl;
#endif
    static const size_t		DATA	= (B/5+(B%5?1:0));
    static const size_t		PARI	= DATA*P/100+1;
    static const size_t		PARITY	= DATA*P/100+1;
    ezpwd::corrector<DATA+PARI>	correct;

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
