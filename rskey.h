#ifndef _EZPWD_RSKEY_H
#define _EZPWD_RSKEY_H

#if defined( __cplusplus )
extern "C" {
#endif

    // ABCDE-FG
    int rskey_2_encode( size_t rawsiz, char *buf, size_t buflen, size_t bufsiz, size_t sep );
    int rskey_2_decode( size_t rawsiz, char *buf, size_t buflen, size_t bufsiz );

    // ABCDE-FGH
    int rskey_3_encode( size_t rawsiz, char *buf, size_t buflen, size_t bufsiz, size_t sep );
    int rskey_3_decode( size_t rawsiz, char *buf, size_t buflen, size_t bufsiz );

    // ABCDE-FGH1
    int rskey_4_encode( size_t rawsiz, char *buf, size_t buflen, size_t bufsiz, size_t sep );
    int rskey_4_decode( size_t rawsiz, char *buf, size_t buflen, size_t bufsiz );

    // ABCDE-FGH1K
    int rskey_5_encode( size_t rawsiz, char *buf, size_t buflen, size_t bufsiz, size_t sep );
    int rskey_5_decode( size_t rawsiz, char *buf, size_t buflen, size_t bufsiz );


#if defined( __cplusplus )
} // extern "C"
#endif

#endif // _EZPWD_RSKEY_H
