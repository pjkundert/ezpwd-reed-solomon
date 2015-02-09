#ifndef _EZPWD_RSPWD_H
#define _EZPWD_RSPWD_H

#if defined( __cplusplus )
extern "C" {
#endif

    size_t rspwd_encode_1( char *p, size_t s );
    size_t rspwd_encode_2( char *p, size_t s );
    size_t rspwd_encode_3( char *p, size_t s );
    size_t rspwd_encode_4( char *p, size_t s );
    size_t rspwd_encode_5( char *p, size_t s );

#if defined( __cplusplus )
} // extern "C"
#endif

#endif // _EZPWD_RSPWD_H
