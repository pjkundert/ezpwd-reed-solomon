
#include <iostream>
#include <utility>

#include <ezpwd/ezcod>		// C++ implementation
#if defined( DEBUG )
#  include <ezpwd/output>
#endif // DEBUG

#include <ezpwd/definitions>	// must be included in one C++ compilation unit

#include "ezcod.h"		// C API declarations

// 
// Encode lat/lon with default worst-case 3m. accuracy: 1 part in 2^22 Latitude, 2^23 Longitude.
// 
template < size_t P=1 >
int				ezcod_3_encode(
				    double		lat,
				    double		lon,
				    char	       *enc,
				    size_t		siz,
				    size_t		pre	= 0 ) // precision (0 --> default 9)
{
#if defined( DEBUG ) && DEBUG > 1
    std::cout
	<< "ezcod_3_encode<"	<< P << ">("
	<< " lat == "		<< lat
	<< ", lon == "		<< lon
	<< ", w/ "		<< pre << " symbols precision"
#if DEBUG > 2
	<< ", buf[" << siz << "] == " << std::vector<uint8_t>( (uint8_t*)enc, (uint8_t*)enc + siz )
#endif
	<< std::endl;
#endif
    int				res;
    try {
	const std::string      &str( ezpwd::ezcod<P>( lat, lon ).encode( pre ));
	if ( str.size() + 1 > siz )
	    throw std::runtime_error( "insufficient buffer provided" );
	std::copy( str.begin(), str.end(), enc );
	enc[str.size()]			= 0;
	res				= str.size();
    } catch ( std::exception &exc ) {
	ezpwd::streambuf_to_buffer sbf( enc, siz );
	std::ostream( &sbf )
	    << "ezcod_3_encode<" << P << ">(" << lat << ", " << lon << ") failed: "
	    << exc.what();
	res				= -1;
    }
    return res;
}

// 
// Decode lat/lon position in degrees and (optionally) accuracy in m, returning confidence in %.
// 
template < size_t P=1 >
int				ezcod_3_decode(
				    char	       *dec,
				    size_t		siz,
				    double	       *lat	= 0,
				    double	       *lon	= 0,
				    double	       *acc	= 0 )
{
    int				confidence;
    try {
	ezpwd::ezcod<P>		decoder( dec );
	confidence			= decoder.confidence;
#if defined( DEBUG ) && DEBUG > 1
	std::cout
	    << "ezcod_3_decode<" << P << ">(\"" << dec
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
	std::string		location( dec );
	ezpwd::streambuf_to_buffer sbf( dec, siz );
	std::ostream( &sbf )
	    << "ezcod_3_decode<" << P << ">(\"" << location << "\") failed: "
	    << exc.what();
	confidence			= -1;
    }
    return confidence;
}

extern "C" {

    /* ezcod 3:10 -- 9+1 Reed-Solomon parity symbol */
    int ezcod_3_10_encode( double lat, double lon, char *enc, size_t siz, size_t pre )
    {
	return ezcod_3_encode<1>( lat, lon, enc, siz, pre );
    }
    int ezcod_3_10_decode( char *dec, size_t siz, double *lat, double *lon, double *acc )
    {
	return ezcod_3_decode<1>( dec, siz, lat, lon, acc );
    }

    /* ezcod 3:11 -- 9+2 Reed-Solomon parity symbols */
    int ezcod_3_11_encode( double lat, double lon, char *enc, size_t siz, size_t pre )
    {
	return ezcod_3_encode<2>( lat, lon, enc, siz, pre );
    }
    int ezcod_3_11_decode( char *dec, size_t siz, double *lat, double *lon, double *acc )
    {
	return ezcod_3_decode<2>( dec, siz, lat, lon, acc );
    }

    /* ezcod 3:12 -- 9+3 Reed-Solomon parity symbols */
    int ezcod_3_12_encode( double lat, double lon, char *enc, size_t siz, size_t pre )
    {
	return ezcod_3_encode<3>( lat, lon, enc, siz, pre );
    }
    int ezcod_3_12_decode( char *dec, size_t siz, double *lat, double *lon, double *acc )
    {
	return ezcod_3_decode<3>( dec, siz, lat, lon, acc );
    }

} // extern "C"

#if defined( __cheerp )
int webMain()
{
    return 0;
}
#endif
