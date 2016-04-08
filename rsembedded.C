// 
// rsembedded.C
// 
//     Example of using ezpwd/rs in an embedded environment.  Such environments often have certain
// constraints:
// 
//   o No exception support
//   o Limited memory
// 
//     Therefore, no STL containers with dynamic allocation may be used, and -fno-exceptions may be
// specified if EZPWD_NO_EXCEPTS is defined.
//
#include <cstdio>
#include <cctype>
#include <array>

#include <ezpwd/rs>
#include <ezpwd/output>

int main()
{
    int				failures= 0;
    ezpwd::RS<255,253>		rs;

    // Use of fixed-size container
    std::fputs( "\n\nFixed-size container:\n", stdout );
    std::array<uint8_t,15>	raw = { 'H', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!', 'x', 'x' }; // working data, w/ space for parity
    std::fputs( "Original:  ", stdout ); ezpwd::hexout( raw.begin(), raw.end(), stdout ); fputc( '\n', stdout );
    rs.encode( raw );				// 13 symbols data + 2 symbols R-S parity added
    std::fputs( "Encoded:   ", stdout ); ezpwd::hexout( raw.begin(), raw.end(), stdout ); fputc( '\n', stdout );
    raw[3]				= 'x';	// Corrupt one symbol
    std::fputs( "Corrupted: ", stdout ); ezpwd::hexout( raw.begin(), raw.end(), stdout ); fputc( '\n', stdout );
    int				count	= rs.decode( raw );  // Correct any symbols possible
    if ( count != 1 )
        failures		       += 1;
    std::fputs( "Corrected: ", stdout ); ezpwd::hexout( raw.begin(), raw.end(), stdout );
    std::fputs( " : ", stdout ); std::fputc( '0'+count, stdout );
    std::fputs( " errors fixed", stdout ); fputc( '\n', stdout );
    // Ensure original data is recovered (ignoring parity)
    for ( auto i = 0UL; i < raw.size() - 2; ++i ) {
        if ( raw[i] != "Hello, world!"[i] ) {
	    failures		       += 1;
	    std::fputs( "Failed to restore origin data.\n", stdout );
	    break;
	}
    }

    return failures ? 1 : 0;
}
