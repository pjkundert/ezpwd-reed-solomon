
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
	std::cout << "Original:  " << std::vector<uint8_t>( copy.begin(), copy.end() ) << std::endl;
	rs.encode( copy );				// 13 symbols copy + 2 symbols R-S parity added
	std::cout << "Encoded:   " << std::vector<uint8_t>( copy.begin(), copy.end() ) << std::endl;
	copy[3]				= 'x';	// Corrupt one symbol
	std::cout << "Corrupted: " << std::vector<uint8_t>( copy.begin(), copy.end() ) << std::endl;
	int			count	= rs.decode( copy );  // Correct any symbols possible
	std::cout << "Corrected: " << std::vector<uint8_t>( copy.begin(), copy.end() ) << " : " << count << " errors fixed" << std::endl;
	copy.resize( copy.size() - rs.nroots() );	// Discard added R-S parity symbols
	std::cout << "Restored:  " << std::vector<uint8_t>( copy.begin(), copy.end() ) << std::endl;
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
	if ( i & 1 ) {
	    erasure.push_back( i );
	    data[i]			= ' ';		// erasure
	    std::cout << "Erasure:   " << std::vector<uint8_t>( data.begin(), data.end() ) << std::endl;
	} else {
	    data[i]		       ^= 1 << i % 8;	// error
	    std::cout << "Corrupted: " << std::vector<uint8_t>( data.begin(), data.end() ) << std::endl;
	}
	std::vector<int>	position;
	int 			count	= rs.decode( data, erasure, &position );
	std::string		fixes( data.size() * 2, ' ' );
	for ( int i : position )
	    fixes[i*2+0] = fixes[i*2+1]	= '^';
	std::cout << "Fixed:     " << fixes << "(count: " << count << ")" << std::endl;
	std::cout << "Decoded:   " << std::vector<uint8_t>( data.begin(), data.end() ) << std::endl << std::endl;

	// Remove R-S parity symbols, ensure original copy is recovered
	data.resize( data.size() - rs.nroots() );
	if ( std::string( data.begin(), data.end() ) != orig ) {
	    failures		       += 1;
	    std::cout << "Failed to restore origin data." << std::endl;
	}
    }

    // Use of fixed-size container
    std::cout << std::endl << std::endl << "Fixed-size container:" << std::endl;
    std::array<uint8_t,15>	raw = { 'H', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!', 'x', 'x' }; // working data, w/ space for parity
    std::cout << "Original:  " << raw << std::endl;
    rs.encode( raw );				// 13 symbols data + 2 symbols R-S parity added
    std::cout << "Encoded:   " << raw << std::endl;
    raw[3]				= 'x';	// Corrupt one symbol
    std::cout << "Corrupted: " << raw << std::endl;
    int				count	= rs.decode( raw );  // Correct any symbols possible
    std::cout << "Corrected: " << raw << " : " << count << " errors fixed" << std::endl;
    // Ensure original data is recovered (ignoring parity)
    if ( std::string( raw.begin(), raw.begin() + 13 ) != orig ) {
	failures		       += 1;
	std::cout << "Failed to restore origin data." << std::endl;
    }

    return failures ? 1 : 0;
}
