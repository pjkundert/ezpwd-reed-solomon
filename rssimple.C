
#include <iostream>
#include <iomanip>
#include <cctype>
#include <cstring>
#include <sys/time.h>
#include <array>
#include <vector>

#include <rs>

// 
// timeofday -- Return current time.
// epoch     -- The UNIX epoch.
// <timeval> < <timeval> -- less-than comparison on timevals
// <timeval> - <timeval> -- difference on timevals
// 
inline
timeval				timeofday()
{
    timeval			tv;
    ::gettimeofday( &tv, NULL );
    return tv;
}

timeval				epoch()
{
    timeval			tv;
    tv.tv_sec				= 0;
    tv.tv_usec				= 0;
    return tv;
}

inline
bool				operator<(
				    const timeval      &lhs,
				    const timeval      &rhs )
{
    return ( lhs.tv_sec 		<  rhs.tv_sec
	     || (( lhs.tv_sec		== rhs.tv_sec )
		 && ( lhs.tv_usec 	<  rhs.tv_usec )));
}

inline
timeval				operator-(
				    const timeval      &lhs,
				    timeval             rhs ) // copy; adjusted...
{
    timeval			result;
    if ( lhs < rhs ) {
	result				= epoch();
    } else {
	// See http://www.gnu.org/software/libc/manual/html_node/Elapsed-Time.html
	if ( lhs.tv_usec < rhs.tv_usec ) {
	    int 		sec	= ( rhs.tv_usec - lhs.tv_usec ) / 1000000 + 1;
	    rhs.tv_usec		       -= sec * 1000000;
	    rhs.tv_sec		       += sec;
	}
	if ( lhs.tv_usec - rhs.tv_usec > 1000000 ) {
	    int 		sec	= ( lhs.tv_usec - rhs.tv_usec ) / 1000000;
	    rhs.tv_usec 	       += sec * 1000000;
	    rhs.tv_sec		       -= sec;
	}
	result.tv_sec			= lhs.tv_sec  - rhs.tv_sec;
	result.tv_usec			= lhs.tv_usec - rhs.tv_usec;
    }
    return result;
}

inline
double				microseconds( const timeval &rhs )
{
    return rhs.tv_usec / 1000000.0 + rhs.tv_sec;
}


int main() 
{
    typedef RS_255( 253 )	rs_255_253;
    rs_255_253			rs;
    std::string			orig	= "Hello, world!";
    std::vector<int>		erasure;

    rs.encode( orig );
    std::cout << "Encoded: " << std::vector<uint8_t>( orig.begin(), orig.end() ) << std::endl;

    for ( size_t i = 0; i < orig.size(); ++i ) {
	std::string		data( orig );
	erasure.clear();
	if ( i & 1 ) {
	    erasure.push_back( i );
	    data[i]			= ' ';		// erasure
	    std::cout << "Erasure: " << std::vector<uint8_t>( data.begin(), data.end() ) << std::endl;
	} else {
	    data[i]		       ^= 1 << i % 8;	// error
	    std::cout << "Corrupt: " << std::vector<uint8_t>( data.begin(), data.end() ) << std::endl;
	}
	int 			count	= rs.decode( data, &erasure );
	std::string		fixes( data.size() * 2, ' ' );
	for ( int i : erasure )
	    fixes[i*2+0] = fixes[i*2+1]	= '^';
	std::cout << "Fixed:   " << fixes << "(count: " << count << ")" << std::endl;
	std::cout << "Decoded: " << std::vector<uint8_t>( data.begin(), data.end() ) << std::endl << std::endl;
    }

    // Get a basic TPS rate for a simple R-S decode with an error
    timeval		beg	= timeofday();
    timeval		end	= beg;
    end.tv_sec		       += 1;
    int			count	= 0;
    timeval		now;
    while (( now = timeofday() ) < end ) {
	for ( int final = count + 1000; count < final; ++count ) {
	    std::string		data( orig );
	    data[0] ^= 1;
	    rs.decode( data );
	}
    }
    double		elapsed	= microseconds( now - beg );
    std::cout 
	<< rs << " rate: "
	<< count / elapsed / 1000 << " kTPS."
	<< std::endl;
}
