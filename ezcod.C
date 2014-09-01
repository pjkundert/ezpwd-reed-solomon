
#include <iostream>
#include <utility>

#include <ezpwd/ezcod>		// C++ implementation
#include "ezcod.h"		// C API declarations
#if defined( DEBUG )
#include <ezpwd/output>
#endif

// 
// <buf_t> << <string> -- Copy the <string> into the C <char*,size_t> buffer, always NUL terminating
// 
//     Copies <string> contents into buffer, and always NUL-terminates.  Returns advanced buf_t (NOT
// including the terminating NUL, suitable for repeating ... << <string> operations.
// 
typedef std::pair<char *,size_t> buf_t;
buf_t			operator<<(
			    const buf_t	       &buf,
			    const std::string  &str )
{
    if ( buf.first && str.size() + 1 <= buf.second ) {
	std::copy( str.begin(), str.end(), buf.first );
	buf.first[str.size()]		= 0;
	return buf_t( buf.first + str.size(), buf.second - str.size() );
    } else if ( buf.first && buf.second ) {
	std::copy( str.begin(), str.begin() + buf.second - 1, buf.first );
	buf.first[buf.second-1]		= 0;
	return buf_t( buf.first + buf.second - 1, 1 );
    }
    return buf; // NULL pointer or 0 size.
}

// 
// Encode lat/lon with default worst-case 3m. accuracy: 1 part in 2^22 Latitude, 2^23 Longitude.
// 
template < size_t P=1, size_t L=9 >
int				ezcod_3_encode(
				    double		lat,
				    double		lon,
				    char	       *enc,
				    size_t		siz )
{
#if defined( DEBUG ) && DEBUG > 1
    std::cout
	<< "ezcod_3_encode<" << P << "," << L << ">("
	<< " lat == " << lat
	<< ", lon == " << lon
#if DEBUG > 2
	<< ", buf[" << siz << "] == " << std::vector<uint8_t>( (uint8_t*)enc, (uint8_t*)enc + siz )
#endif
	<< std::endl;
#endif
    int				res;
    std::string			str;
    try {
	ezpwd::ezcod<P,L>	loc( lat, lon );
	str				= loc.encode();
	res				= str.size();
	if ( str.size() + 1 > siz )
	    throw std::runtime_error( "ezcod_3_encode: insufficient buffer provided" );
	res				= str.size();
    } catch ( std::exception &exc ) {
	str				= exc.what();
	res				= -1;
    }
    buf_t( enc, siz ) << str;
    return res;
}

// 
// Decode lat/lon position in degrees and (optionally) accuracy in m, returning confidence in %.
// 
template < size_t P=1, size_t L=9 >
int				ezcod_3_decode(
				    char	       *dec,
				    size_t		siz,
				    double	       *lat	= 0,
				    double	       *lon	= 0,
				    double	       *acc	= 0 )
{
    int				res;
    std::string			str;
    try {
	ezpwd::ezcod<P,L>	loc;
	res				= loc.decode( dec );
#if defined( DEBUG ) && DEBUG > 1
    std::cout
	<< "ezcod_3_decode<" << P << "," << L << ">("
	<< " lat (" << (void *)lat << ") == " << loc.latitude
	<< ", lon (" << (void *)lon << ") == " << loc.longitude
	<< ", acc (" << (void *)acc << ") == " << loc.accuracy
	<< std::endl;
#endif
	if ( lat )
	    *lat			= loc.latitude;
	if ( lon )
	    *lon			= loc.longitude;
	if ( acc )
	    *acc			= loc.accuracy;
    } catch ( std::exception &exc ) {
	str				= exc.what();
	res				= -1;
    }
    buf_t( dec, siz ) << str;
    return res;
}


extern "C" {

    /* ezcod 3:10 -- 9+1 Reed-Solomon parity symbol */
    int ezcod_3_10_encode( double lat, double lon, char *enc, size_t siz )
    {
	return ezcod_3_encode<1>( lat, lon, enc, siz );
    }
    int ezcod_3_10_decode( char *dec, size_t siz, double *lat, double *lon, double *acc )
    {
	return ezcod_3_decode<1>( dec, siz, lat, lon, acc );
    }

    /* ezcod 3:11 -- 9+2 Reed-Solomon parity symbols */
    int ezcod_3_11_encode( double lat, double lon, char *enc, size_t siz )
    {
	return ezcod_3_encode<2>( lat, lon, enc, siz );
    }
    int ezcod_3_11_decode( char *dec, size_t siz, double *lat, double *lon, double *acc )
    {
	return ezcod_3_decode<2>( dec, siz, lat, lon, acc );
    }

    /* ezcod 3:12 -- 9+3 Reed-Solomon parity symbols */
    int ezcod_3_12_encode( double lat, double lon, char *enc, size_t siz )
    {
	return ezcod_3_encode<3>( lat, lon, enc, siz );
    }
    int ezcod_3_12_decode( char *dec, size_t siz, double *lat, double *lon, double *acc )
    {
	return ezcod_3_decode<3>( dec, siz, lat, lon, acc );
    }

} // extern "C"
