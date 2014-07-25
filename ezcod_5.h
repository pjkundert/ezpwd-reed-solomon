#ifndef _EZPWD_EZCOD_H
#define _EZPWD_EZCOD_H

/*
 * "C" API for ezcod_5, with 1 to or 3 parity symbols.
 * 
 * ..._encode -- Encode the lat/lon into an ezcod 5:10 string in buffer supplied, returning length
 * ..._decode -- Decode the ezcode 5:10 string into lat, lon, returning confidence percentage
 * 
 *     All return -'ve value on failure, and as much of an error message as allowed by the 'siz' of
 * the supplied char* buffer.
 */
#if defined( __cplusplus )
extern "C" {
#endif

    /* ezcod 5:10 -- 9+1 Reed-Solomon parity symbol */
    int ezcod_5_10_encode( double  lat, double  lon, char *enc, size_t siz );
    int ezcod_5_10_decode( char *dec, size_t siz, double *lat, double *lon, double *acc );

    /* ezcod 5:11 -- 9+2 Reed-Solomon parity symbols */
    int ezcod_5_11_encode( double  lat, double  lon, char *enc, size_t siz );
    int ezcod_5_11_decode( char *dec, size_t siz, double *lat, double *lon, double *acc );

    /* ezcod 5:12 -- 9+3 Reed-Solomon parity symbols */
    int ezcod_5_12_encode( double  lat, double  lon, char *enc, size_t siz );
    int ezcod_5_12_decode( char *dec, size_t siz, double *lat, double *lon, double *acc );

#if defined( __cplusplus )
} // extern "C"
#endif

#endif // _EZPWD_EZCOD_H

