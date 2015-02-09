#ifndef _EZPWD_EZCOD_H
#define _EZPWD_EZCOD_H

/*
 * "C" API for ezcod w/3m accuracy in 9 symbols, with 1, 2 or 3 parity symbols.
 * 
 * ..._encode -- Encode the lat/lon into an ezcod 3:10 string in buffer supplied, returning length
 * ..._decode -- Decode the ezcode 3:10 string into lat, lon, returning confidence percentage
 * 
 *     All return -'ve value on failure, and as much of an error message as allowed by the 'siz' of
 * the supplied char* buffer.
 */
#  if defined( __cplusplus )
// Standard C Javascript binding implementation
extern "C" {
#  endif
    /* ezcod 3:10 -- 9+1 Reed-Solomon parity symbol */
    int				ezcod_3_10_encode(
				    double		lat,
				    double		lon,
				    char	       *enc,
				    size_t		siz,
				    size_t		pre );
    int				ezcod_3_10_decode(
				    char	       *dec,
				    size_t		siz,
				    double	       *lat,
				    double	       *lon,
				    double	       *acc );
    /* ezcod 3:11 -- 9+2 Reed-Solomon parity symbols */
    int				ezcod_3_11_encode(
				    double		lat,
				    double		lon,
				    char	       *enc,
				    size_t		siz,
				    size_t		pre );
    int				ezcod_3_11_decode(
				    char	       *dec,
				    size_t		siz,
				    double	       *lat,
				    double	       *lon,
				    double	       *acc );
    /* ezcod 3:12 -- 9+3 Reed-Solomon parity symbols */
    int				ezcod_3_12_encode(
				    double		lat,
				    double		lon,
				    char	       *enc,
				    size_t		siz,
				    size_t		pre );
    int				ezcod_3_12_decode(
				    char	       *dec,
				    size_t		siz,
				    double	       *lat,
				    double	       *lon,
				    double	       *acc );
#  if defined( __cplusplus )
} // extern "C"

#    if defined( __cheerp )
// Cheerp C++ Javascript binding implementation
class [[jsexport]] ezcod_cheerp
{
  public:
				ezcod_cheerp()
				{
				    ;
				}

    /* ezcod 3:10 -- 9+1 Reed-Solomon parity symbol */
    int				ezcod_3_10_encode(
				    double		lat,
				    double		lon,
				    char	       *enc,
				    size_t		siz,
				    size_t		pre )
    {
	return ::ezcod_3_10_encode( lat, lon, enc, siz, pre );
    }
    int				ezcod_3_10_decode(
				    char	       *dec,
				    size_t		siz,
				    double	       *lat,
				    double	       *lon,
				    double	       *acc )
    {
	return ::ezcod_3_10_decode( dec, siz, lat, lon, acc );
    }
    /* ezcod 3:11 -- 9+2 Reed-Solomon parity symbols */
    int				ezcod_3_11_encode(
				    double		lat,
				    double		lon,
				    char	       *enc,
				    size_t		siz,
				    size_t		pre )
    {
	return ::ezcod_3_11_encode( lat, lon, enc, siz, pre );
    }
    int				ezcod_3_11_decode(
				    char	       *dec,
				    size_t		siz,
				    double	       *lat,
				    double	       *lon,
				    double	       *acc )
    {
	return ::ezcod_3_11_decode( dec, siz, lat, lon, acc );
    }
    /* ezcod 3:12 -- 9+3 Reed-Solomon parity symbols */
    int				ezcod_3_12_encode(
				    double		lat,
				    double		lon,
				    char	       *enc,
				    size_t		siz,
				    size_t		pre )
    {
	return ::ezcod_3_12_encode( lat, lon, enc, siz, pre );
    }
    int				ezcod_3_12_decode(
				    char	       *dec,
				    size_t		siz,
				    double	       *lat,
				    double	       *lon,
				    double	       *acc )
    {
	return ::ezcod_3_12_decode( dec, siz, lat, lon, acc );
    }
};
#    endif // __cheerp
#  endif // __cplusplus
#endif // _EZPWD_EZCOD_H
