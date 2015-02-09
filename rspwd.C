
#include <ezpwd/corrector>

#include <ezpwd/definitions>	// must be included in one C++ compilation unit

#include "rspwd.h"		// C API declarations

// 
// Implementation of "C" rspwd_encode_<N> API, as described by rspwd.h
// 
extern "C" {
    size_t rspwd_encode_1( char *p, size_t s ) { return ezpwd::corrector<1>::encode( p, s ); }
    size_t rspwd_encode_2( char *p, size_t s ) { return ezpwd::corrector<2>::encode( p, s ); }
    size_t rspwd_encode_3( char *p, size_t s ) { return ezpwd::corrector<3>::encode( p, s ); }
    size_t rspwd_encode_4( char *p, size_t s ) { return ezpwd::corrector<4>::encode( p, s ); }
    size_t rspwd_encode_5( char *p, size_t s ) { return ezpwd::corrector<5>::encode( p, s ); }
} // extern "C"

