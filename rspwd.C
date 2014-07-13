// -*- coding: utf-8 -*-

#include <string.h>

#include <rs>
#include <rspwd.h>

extern "C" {
    size_t rspwd_encode_1( char *p, size_t s ) { return ezpwd::corrector<1>::encode( p, s ); }
    size_t rspwd_encode_2( char *p, size_t s ) { return ezpwd::corrector<2>::encode( p, s ); }
    size_t rspwd_encode_3( char *p, size_t s ) { return ezpwd::corrector<3>::encode( p, s ); }
    size_t rspwd_encode_4( char *p, size_t s ) { return ezpwd::corrector<4>::encode( p, s ); }
    size_t rspwd_encode_5( char *p, size_t s ) { return ezpwd::corrector<5>::encode( p, s ); }
} // extern "C"

#if defined(  TEST )

#include <iostream>
#include <sstream>

template < size_t N >
inline int			rspwd_test( std::ostream &failmsgs )
{
    char		        pwd[] = "Mag.1ckπ";
    char			enc[N + sizeof pwd];
    int				failures= 0;
    
    strncpy( enc, pwd, sizeof enc );
    size_t			len	= ezpwd::corrector<N>::encode( enc, sizeof enc );
    failmsgs
	<< "expwd::corrector<"	<< N 
	<< ">: Password: \""	<< pwd 
	<< "\" ==> \""		<< enc
	<< "\""
	<< std::endl;
    if ( len != sizeof enc - 1 ) {
	failmsgs << "Expected N == " << N << " parity symbols" << std::endl;
	++failures;
    }

    // 0) Full parity.  Should be able to handle up to N/2-1 errors in the low 6 bits.
    for ( int e = 0; e <= N/2; ++e ) {
	char			err[sizeof enc + N]; // be prepared for up to N extra symbols
	std::copy( enc, enc + sizeof enc, err );
	for ( int i = 0; i < e; ++i )
	    err[i]	       	       ^= 1<<(i%4); // flip one of the 3 low bits, eg. entry error
	char			fix[sizeof err];
	std::copy( err, err + sizeof err, fix );
	
	int			confidence = ezpwd::corrector<N>::decode( fix, sizeof fix );
	failmsgs
	    << " w/"		<< e
	    << " errors; \""	<< err
	    << "\" ==> \""	<< fix
	    << " w/ " 		<< std::setw( 3 ) << confidence
	    << "% confidence"
	    << std::endl;
	++failures;
    }
    return failures;
}

std::ostream		       &operator<<(
				    std::ostream       &lhs,
				    std::ostringstream &rhs )
{
    return lhs << rhs.str();
}

int				main()
{
#if defined( DEBUG ) && DEBUG > 0
    std::ostream	       &failmsgs = std::cout;
#else
    std::ostringstream		failmsgs;
#endif
    int				failures= 0;
    failures			       += rspwd_test<1>( failmsgs );
    failures			       += rspwd_test<2>( failmsgs );
    failures			       += rspwd_test<3>( failmsgs );
    failures			       += rspwd_test<4>( failmsgs );
    failures			       += rspwd_test<5>( failmsgs );
#if ! defined( DEBUG ) || DEBUG == 0
    if ( failures )
	std::cout
	    << __FILE__ << " fails " << failures << " tests: "
	    << std::endl
	    << failmsgs
	    << std::endl
	    << std::endl;
#endif
    return failures ? 1 : 0;
}
#endif
