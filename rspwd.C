// -*- coding: utf-8 -*-

#include <string.h>

#include <rs>
#include <rspwd.h>

namespace ezpwd {

    template < size_t N >
    struct rspwd {
	static ezpwd::corrector<N> correct;
	// 
	// ezpwd::rspwd::encode -- append N base-64 parity symbols to password
	// 
	//     The supplied password buffer size must be sufficient to contain N additional symbols, plus
	// the terminating NUL.  Returns the resultant encoded password size (excluding the NUL).
	// 
	static size_t		encode(
				    char       	       *password,
				    size_t		size )	// maximum available size
	{
	    size_t		len	= ::strlen( password );	// length w/o terminating NUL
	    if ( len + N + 1 > size )
		throw std::runtime_error( "ezpwd::rspwd::encode password buffer has insufficient capacity" );
	    std::string		parity	= correct.parity( std::string( password, password + len ));
	    if ( parity.size() != N )
		throw std::runtime_error( "ezpwd::rspwd::encode computed parity with incorrect size" );
	    std::copy( parity.begin(), parity.end(), password + len );
	    len			       += N;
	    password[len]		= 0;
	    return len;
	}
    };

    // 
    // Define the static rspwd<...> members; allowed in header for template types.
    // 
    template < size_t N >
    ezpwd::corrector<N>		rspwd<N>::correct;

} // namespace ezpwd

extern "C" {
    size_t rspwd_encode_1( char *p, size_t s ) { return ezpwd::rspwd<1>::encode( p, s ); }
    size_t rspwd_encode_2( char *p, size_t s ) { return ezpwd::rspwd<2>::encode( p, s ); }
    size_t rspwd_encode_3( char *p, size_t s ) { return ezpwd::rspwd<3>::encode( p, s ); }
    size_t rspwd_encode_4( char *p, size_t s ) { return ezpwd::rspwd<4>::encode( p, s ); }
    size_t rspwd_encode_5( char *p, size_t s ) { return ezpwd::rspwd<5>::encode( p, s ); }
    } // extern "C"

#if defined(  TEST )

#include <iostream>
#include <sstream>

template < size_t N >
inline int			rspwd_test( std::ostream &failmsgs )
{
    char		        password[] = "Mag.1ckπ";
    char			tmp[N + sizeof password];
    int				failures= 0;

    std::string			parity;
    ezpwd::rspwd<N>::correct.rscodec.encode( password, parity );
    failmsgs << std::vector<uint8_t>( parity.begin(), parity.end() ) << std::endl;

    strncpy( tmp, password, sizeof tmp );
    size_t			len	= ezpwd::rspwd<N>::encode( tmp, sizeof tmp );
    failmsgs
	<< "expwd::rspwd<"	<< N 
	<< ">: Password: \""	<< password 
	<< "\" ==> \""		<< tmp
	<< "\""
	<< std::endl;
    if ( len != sizeof tmp - 1 ) {
	failmsgs << "Expected N == " << N << " parity symbols" << std::endl;
	++failures;
    }

    return failures;
}
    
int				main()
{
    std::ostringstream		failmsgs;
    int				failures= 0;
    failures			       += rspwd_test<1>( failmsgs );
    failures			       += rspwd_test<2>( failmsgs );
    failures			       += rspwd_test<3>( failmsgs );
    failures			       += rspwd_test<4>( failmsgs );
    failures			       += rspwd_test<5>( failmsgs );
    if ( failures )
	std::cout
	    << __FILE__ << " fails " << failures << " tests: "
	    << std::endl
	    << failmsgs.str()
	    << std::endl
	    << std::endl;
    return failures ? 1 : 0;
}
#endif
