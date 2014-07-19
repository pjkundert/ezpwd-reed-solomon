
#include <iostream>
#include <iomanip>
#include <cctype>
#include <cstring>
#include <sys/time.h>
#include <array>
#include <vector>

#include <ezpwd/rs>
#include <ezpwd/timeofday>

int main() 
{
    RS_255( 253 )		rs;
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
    timeval		beg	= ezpwd::timeofday();
    timeval		end	= beg;
    end.tv_sec		       += 1;
    int			count	= 0;
    timeval		now;
    while (( now = ezpwd::timeofday() ) < end ) {
	for ( int final = count + 1000; count < final; ++count ) {
	    std::string		data( orig );
	    data[0] ^= 1;
	    rs.decode( data );
	}
    }
    double		elapsed	= ezpwd::seconds( now - beg );
    std::cout 
	<< rs << " rate: "
	<< count / elapsed / 1000 << " kTPS."
	<< std::endl;
}
