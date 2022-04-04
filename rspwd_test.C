// -*- coding: utf-8 -*-

#include <iostream>
#include <sstream>
#include <string.h>

#include <ezpwd/rs>
#include <ezpwd/corrector>

#include <ezpwd/definitions>	// must be included in one C++ compilation unit

#pragma GCC diagnostic ignored "-Wstringop-truncation" // We know about the truncation of 'password'

// 
// rspwd_test -- test instances of ezpwd::corrector<N>, as used by "C" rspwd_decode_N API
// 
template < size_t P, size_t N >
inline int			rspwd_test( std::ostream &failmsgs, const char *password )
{
    int				failures= 0;

    // Get a buffer just big enough for the password (not including NUL) plus N
    // parity.  We'll copy the NUL-terminated password in there (the NUL using
    // one of the parity spots).
    char			enc[P + N];
    strncpy( enc, password, sizeof enc );
    size_t			len	= ezpwd::corrector<N>::encode( enc, sizeof enc );
    failmsgs
	<< "expwd::corrector<"	<< N 
	<< ">: Password: \""	<< password 
	<< "\" ==> \""		<< enc
	<< "\""
	<< std::endl;
    if ( len != sizeof enc - 1 ) {
	failmsgs << "FAILED: Expected N == " << N << " parity symbols" << std::endl;
	++failures;
    }

    // 0) Full parity.  Should be able to handle up to (N-1)/2 errors in the low 6 bits.  In other
    // words, until you have 3 parity symbols, you cannot withstand a random error and have any
    // reserve parity to gain any certainty that the result is actually correct.
    for ( size_t e = 0; e <= N/2; ++e ) {
	char			err[sizeof enc + N]; // be prepared for up to N extra symbols
	std::copy( enc, enc + sizeof enc, err );
	for ( size_t i = 0; i < e; ++i )
	    err[i]	       	       ^= 1<<(i%4); // flip one of the 3 low bits, eg. entry error
	char			fix[sizeof err];
	std::copy( err, err + sizeof err, fix );
	
	int			confidence = ezpwd::corrector<N>::decode( fix, sizeof fix );
	bool			matched	= !strcmp( fix, password );
	bool			guessed	= confidence > 0 && matched;
	bool			failed	= guessed != ( e <= (N-1)/2 );
	failmsgs
	    << ( guessed
		 ? "GUESSED"
		 : "default" )
	    << ( matched
		 ? " matching"
		 : " MISMATCH" )
	    << ( failed
		 ? " FAILURE "
		 : " success " )
	    << "Full parity "	<< N
	    << " w/ "		<< e
	    << " errors;      "	<< std::setw( 16 ) << err
	    << " ==> "		<< std::setw( 16 ) << fix
	    << " w/ " 		<< std::setw( 3 ) << confidence
	    << "% confidence"
	    << std::endl;
#if ! ( defined( DEBUG ) && DEBUG >= 1 )
	if ( failed )
#endif
	    ++failures;
    }

    // 1) Part parity (no errors).  Should be able to confirm password with no errors, and (N+1)/2+1
    // to N parity symbols via R-S decoding, and from 1 to (N+1)/2 symbols by simple matching.  Clip
    // off up to all N parity symbols and try decoding.
    for ( size_t e = 1; e <= N; ++e ) {
	char			err[sizeof enc + N]; // be prepared for up to N extra symbols
	std::copy( enc, enc + sizeof enc, err );
	err[strlen( enc ) - e]	= 0;		// clip off last 'e' symbols

        char			fix[sizeof err];
	std::copy( err, err + sizeof err, fix );
	int			confidence = ezpwd::corrector<N>::decode( fix, sizeof fix );
	bool			matched	= !strcmp( fix, password );
	bool			guessed	= confidence > 0 && matched;
	bool			failed	= guessed != ( e < N ); // with any parity symbols, we'll get some confidence
	failmsgs
	    << ( guessed
		 ? "GUESSED"
		 : "default" )
	    << ( matched
		 ? " matching"
		 : " MISMATCH" )
	    << ( failed
		 ? " FAILURE "
		 : " success " )
	    << "Part parity "	<< N
	    << " w/ "		<< e
	    << " parity skip  "	<< std::setw( 16 ) << err
	    << " ==> "		<< std::setw( 16 ) << fix
	    << " w/ " 		<< std::setw( 3 ) << confidence
	    << "% confidence"
	    << std::endl;
#if ! ( defined( DEBUG ) && DEBUG >= 1 )
	if ( failed )
#endif
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
    std::cout
	<< "rspwd tests ..."
	<< std::endl;

#if defined( DEBUG ) && DEBUG > 0
    std::ostream	       &failmsgs = std::cout;
#else
    std::ostringstream		failmsgs;
#endif
    int				failures= 0;
    failures			       += rspwd_test<10,1>( failmsgs, "Mag.1ckπ" );
    failures			       += rspwd_test<10,2>( failmsgs, "Mag.1ckπ" );
    failures			       += rspwd_test<10,3>( failmsgs, "Mag.1ckπ" );
    failures			       += rspwd_test<10,4>( failmsgs, "Mag.1ckπ" );
    failures			       += rspwd_test<10,5>( failmsgs, "Mag.1ckπ" );
    failures			       += rspwd_test< 7,1>( failmsgs, "sock1t" );
    failures			       += rspwd_test< 7,2>( failmsgs, "sock1t" );
    failures			       += rspwd_test< 7,3>( failmsgs, "sock1t" );
    failures			       += rspwd_test< 7,4>( failmsgs, "sock1t" );
    failures			       += rspwd_test< 7,5>( failmsgs, "sock1t" );
#if ! defined( DEBUG ) || DEBUG == 0
    if ( failures )
	std::cout
	    << __FILE__ << " fails " << failures << " tests: "
	    << std::endl
	    << failmsgs
	    << std::endl
	    << std::endl;
    else
	std::cout
	    << "  ...all tests passed."
	    << std::endl;
#endif
    return failures ? 1 : 0;
}
