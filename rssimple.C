
#include <iostream>
#include <iomanip>
#include <cctype>
#include <cstring>
#include <sys/time.h>
#include <array>
#include <vector>

#include <ezpwd/rs>
#include <ezpwd/timeofday>
#include <ezpwd/output>

int main() 
{
    int				failures= 0;

    ezpwd::RS<255,253>		rs;		// 255 symbol codeword, up to 253 data == 2 symbols parity
    std::string			orig	= "Hello, world!";

    // Most basic use of API to correct an error, in a dynamic container
    std::cout << std::endl << std::endl << "Simple std::string container:" << std::endl;
    {
	std::string		copy	= orig; // working copy, copy of orig
	std::cout << "Original:  " << ezpwd::hexstr( copy ) << std::endl;
	rs.encode( copy );				// 13 symbols copy + 2 symbols R-S parity added
	std::cout << "Encoded:   " << ezpwd::hexstr( copy ) << std::endl;
	copy[3]				= 'x';	// Corrupt one symbol
	std::cout << "Corrupted: " << ezpwd::hexstr( copy ) << std::endl;
	int			count	= rs.decode( copy );  // Correct any symbols possible
	std::cout << "Corrected: " << ezpwd::hexstr( copy ) << " : " << count << " errors fixed" << std::endl;
	copy.resize( copy.size() - rs.nroots() );	// Discard added R-S parity symbols
	std::cout << "Restored:  " << ezpwd::hexstr( copy ) << std::endl;
	if ( copy != orig ) {				// Ensure original copy is recovered
	    failures		       += 1;
	    std::cout << "Failed to restore origin data." << std::endl;
	}
    }

    // Iterate through orig, corrupting or erasing the i'th character, and fixing it.
    std::cout << std::endl << std::endl << "Iterate over std::vector container:" << std::endl;
    for ( size_t i = 0; i < orig.size(); ++i ) {
	std::vector<uint8_t>	data( orig.begin(), orig.end() );
	rs.encode( data );

	std::vector<int>	erasure;
	if ( i & 1 ) {					// every other loop...
	    erasure.push_back( i );
	    data[i]			= ' ';		// erasure
	    std::cout << "Erasure:   " << data << std::endl;
	} else {
	    data[i]		       ^= 1 << i % 8;	// error
	    std::cout << "Corrupted: " << data << std::endl;
	}
	std::vector<int>	position;
	int 			count	= rs.decode( data, erasure, &position );
	std::string		fixes( data.size() * 2, ' ' );
	for ( int i : position )
	    fixes[i*2+0] = fixes[i*2+1]	= '^';
	std::cout << "Fixed:     " << fixes << "(count: " << count << ")" << std::endl;
	std::cout << "Decoded:   " << data << std::endl << std::endl;

	// Remove R-S parity symbols, ensure original copy is recovered
	data.resize( data.size() - rs.nroots() );
	if ( std::string( data.begin(), data.end() ) != orig ) {
	    failures		       += 1;
	    std::cout << "Failed to restore origin data." << std::endl;
	}
    }

    return failures ? 1 : 0;
}
