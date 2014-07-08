//#define DEBUG 2

#include <rs>
#include <iostream>
#include <iomanip>
#include <cctype>
#include <cstring>
#include <array>
#include <vector>

#include <exercise.H>

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
    exercise( rs, 100 );

    exercise( RS_255_CCSDS( 255-2 )(), 100 );
    exercise( RS_255_CCSDS( 255-4 )(), 100 );
    exercise( RS_255_CCSDS( 255-8 )(), 100 );
    exercise( RS_255_CCSDS( 255-16 )(), 100 );
}
