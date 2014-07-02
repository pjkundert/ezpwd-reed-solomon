#define DEBUG 2

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
    uint8_t			data[255] = "Hello, world!";
    int				len	= strlen( (char *)data );
    int 			parity	= rs.nroots;
    int				eras_pos[rs.nroots] = {0};
    int				no_eras	= 0;
    uint8_t			corr[rs.nroots] = {0};

    rs.encode( data, len, data + len );
    std::cout << "Encoded: " << std::vector<uint8_t>( data, data+len+parity ) << std::endl;

    for ( int i = 0; i < len + parity; ++i ) {
	no_eras = 0;
	if ( i&1 ) {
	    eras_pos[no_eras++] = i; // erasure
	    data[i] = ' ';
	    std::cout << "Erasure: " << std::vector<uint8_t>( data, data+len+parity ) << std::endl;
	} else {
	    data[i] ^= 1 << i % 8; // error
	    std::cout << "Corrupt: " << std::vector<uint8_t>( data, data+len+parity ) << std::endl;
	}
	int 			count	= rs.decode( data, len, data + len, eras_pos, 0, corr );
	std::vector<uint8_t>	fixes( len+parity, '\0' );
	for ( int i = 0; i < count; ++i )
	    fixes[eras_pos[i]] = corr[i];
	std::cout << "Correct: " << fixes << " (" << count << ")" << std::endl;
	std::cout << "Decoded: " << std::vector<uint8_t>( data, data+len+parity ) << std::endl;
    }
    exercise( rs );

    exercise( RS_255_CCSDS( 255-2 )() );
    exercise( RS_255_CCSDS( 255-4 )() );
    exercise( RS_255_CCSDS( 255-8 )() );
    exercise( RS_255_CCSDS( 255-16 )() );
}
