#ifndef _EZPWD_RSKEY_H
#define _EZPWD_RSKEY_H

#if defined( __cplusplus )
extern "C" {
#endif

    int rskey_64_15_encode( char *buf, size_t len, size_t siz );
    int rskey_64_15_decode( char *buf, size_t len, size_t siz );

#if defined( __cplusplus )
} // extern "C"
#endif

#endif // _EZPWD_RSKEY_H
