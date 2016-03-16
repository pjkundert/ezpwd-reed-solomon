// 
// rsencode.C
// 
//     Example of using ezpwd/rs to Reed-Solomon encode a file, in chunks.  If 32 bytes of parity is
// selected, then the RS(255,255-32) == RS(255,223) codec would be used.
// 
// SYNOPSIS
// 
//     rsencode [<output> [<input>]]
// 
//     Read <input> (stdin by default), R-S encode, and write to <output> (stdout by default).  Will
// not work, if the same file is 
// 
// EXAMPLES
//     rsencode - # stdin-->stdout: 32 bytes parity, w/128-byte chunks
// 
// NOTE
//     Set RSDECODE C++ preprocessor symbol to "decode", instead of "encode" data.
// 
#include <string>
#include <iostream>
#include <fstream>
#include <memory>

#include <ezpwd/rs>
#include <ezpwd/output>

constexpr size_t		paritysize = 32; // bytes of parity
constexpr size_t		chunksize = 128; // bytes of data chunk (up to 255-parity)
ezpwd::RS<255,255-paritysize>	rs; // eg; paritysize of 32 ==> RS<255,223>

int main( int argc, const char **argv )
{
    // Set 'inp' to std::cin, or a newly opened file (which will be released at end of scope)
    std::unique_ptr<std::ifstream> inpfile;
    if ( argc > 1 and strcmp( argv[1], "-" )) {
	inpfile.reset( new std::ifstream( argv[1], std::ifstream::binary ));
    }
    std::istream	       &inp( inpfile ? *inpfile : std::cin );

    // Set 'out' to std::cout, or a newly removed and opened file (released at end of scope)
    std::unique_ptr<std::ofstream> outfile;
    if ( argc > 2 and strcmp( argv[2], "-" )) {
	std::remove( argv[2] );
	outfile.reset( new std::ofstream( argv[2], std::ifstream::binary ));
    }
    std::ostream		&out( outfile ? *outfile : std::cout );

    // Read, R-S encode/decode and write chunks
    std::string			chunk;
    size_t			inptotal = 0;
    try {
	while ( inp ) { // 'til error/eof
#if defined( RSDECODE )
	    // R-S Decode
	    chunk.resize( chunksize+paritysize );
	    inp.read( &chunk.front(), chunksize+paritysize );
	    chunk.resize( inp.gcount() );
	    inptotal		       += inp.gcount();
	    if ( chunk.size() == 0 )
		continue;
	    if ( chunk.size() < paritysize + 1 )
		throw std::logic_error( std::string()
					<< "Insufficient data for an " 
					<< rs << " encoded chunk" );
	    // R-S decode (raises std::exception on R-S decode failure), then remove parity
	    rs.decode( chunk );
	    chunk.resize( chunk.size() - paritysize );
#else
	    // R-S Encode
	    chunk.resize( chunksize );
	    inp.read( &chunk.front(), chunksize );
	    chunk.resize( inp.gcount() );
	    inptotal		       += inp.gcount();
	    // std::cerr << "Read:  " << chunk.size() << std::endl;
	    if ( chunk.size() == 0 )
		continue;
	    // R-S encode and add paritysize R-S parity symbols to chunk	    
	    rs.encode( chunk );
#endif
	    // std::cerr << "Wrote: " << chunk.size() << std::endl;
	    out << chunk;
	}
    } catch ( std::exception &exc ) {
	std::cerr << "Error after " << inptotal << " bytes: " << exc.what() << std::endl;
	return 1;
    }
    return 0;
}
