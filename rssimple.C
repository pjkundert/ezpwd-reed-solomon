
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
    ezpwd::RS<255,253>		rs;
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
	std::vector<int>	position;
	int 			count	= rs.decode( data, erasure, &position );
	std::string		fixes( data.size() * 2, ' ' );
	for ( int i : position )
	    fixes[i*2+0] = fixes[i*2+1]	= '^';
	std::cout << "Fixed:   " << fixes << "(count: " << count << ")" << std::endl;
	std::cout << "Decoded: " << std::vector<uint8_t>( data.begin(), data.end() ) << std::endl << std::endl;
    }
}
