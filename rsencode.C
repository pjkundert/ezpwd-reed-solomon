// 
// rsencode.C
// 
//     Example of using ezpwd/rs to Reed-Solomon encode a file, in chunks.  If 32 bytes of parity is
// selected, then the RS(255,255-32) == RS(255,223) codec would be used.  If an RS(511,...) or
// larger R-S codec is used (implying 9-bit or larger symbol size), then multiple bytes will be
// used 
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
#include <vector>

#include <ezpwd/rs>
#include <ezpwd/output>

// Choose Reed-Solomon codeword capacity (and symbol bits), parity and default chunk size The
// CODEWORD and PARITY are fixed compile-time constants, but the chunk size (the number of symbols
// of data protected by the parity) can be changed at run-time.
#if ! defined( RSCODEWORD )
#  define RSCODEWORD 255
#endif
#if ! defined( RSPARITY )
#  define RSPARITY 32
#endif
#if ! defined( RSCHUNK )
#  define RSCHUNK 128
#endif

// 
// serialize	-- convert data symbols to N*(sizeof symbol_t()) string of chars
// deserialize	-- convert chunk string to N/(sizeof symbol_t()) symbols
// 
//     Big-endian (most significant bits are output first), supports arbitrary sized symbol_t
// (likely only 2 bytes).  Provide a specialization (not typically recommended practice for function
// templates) to avoid warnings on some compilers for <unsigned char> << 8.
// 
template < typename symbol_t >
std::string			serialize(
				    const std::vector<symbol_t>
						       &data )
{
    std::string			chunk( data.size() * sizeof symbol_t(), char( 0 ));
    for ( size_t s = 0; s < data.size(); ++s ) {
	for ( size_t b = 0; b < sizeof symbol_t(); ++b ) {
	    chunk[s * sizeof symbol_t() + b]
					= uint8_t(( data[s] >> ( sizeof symbol_t() - 1 - b ) * 8 ) & 0xff );
	}
    }
    return chunk;
}

template < typename symbol_t >
std::vector<symbol_t>		deserialize(
				    const std::string  &chunk )
{
    std::vector<symbol_t>	data( chunk.size() / sizeof symbol_t() );
    for ( size_t s = 0; s < data.size(); ++s ) {
	for ( size_t b = 0; b < sizeof symbol_t(); ++b ) {
	    data[s]		      <<= 8;		// NO-OP on first loop (zero)
	    data[s]		       |= uint8_t( chunk[s * sizeof symbol_t() + b] );
	}
    }
    return data;
}
template <>
std::vector<unsigned char>	deserialize<unsigned char>(
				    const std::string  &chunk )
{
    return std::vector<unsigned char>( chunk.begin(), chunk.end() );
}

// 
// encode
// decode
// 
//     Read a chunk of input and process, returning the processed chunk for future use.
// 
template<typename RS_t>
std::string		       &encode(
				    RS_t	       &rs, // R-S codec, and its capacity, parity
				    std::istream       &inp,
				    size_t	       &inptotal,
				    std::string	       &chunk,
				    size_t		chunksize )
{
    typedef typename RS_t::symbol_t symbol_t;

    // R-S Encode.  Read up to chunksize (or to EOF) symbols worth of data
    chunk.resize( chunksize * sizeof symbol_t() );
    inp.read( &chunk.front(), chunksize * sizeof symbol_t() );
    chunk.resize( inp.gcount() );
    inptotal			       += inp.gcount();

    // Make sure either 0 (EOF at start of input) or an even symbol's worth of data was read.
    if ( chunk.size() == 0 )
	return chunk;
    if ( chunk.size() % sizeof symbol_t() )
	throw std::logic_error( std::string() << "Insufficient data for an " << rs << " encoded chunk" );

    // R-S encode (adds rs.NROOTS R-S parity symbols to chunk)
    if ( sizeof symbol_t() == 1 )// <= 8-bit symbols.  Encodes in-place
	rs.encode( chunk );
    else {			// >= 16-bit symbols.  Deserialize to symbols, encode, re-serialize
	auto		data	= deserialize<symbol_t>( chunk );
	rs.encode( data );
	chunk			= serialize<symbol_t>( data );
    }
    return chunk;
}

template <typename RS_t>
std::string		       &decode(
				    RS_t	       &rs,
				    std::istream       &inp,
				    size_t	       &inptotal,
				    std::string	       &chunk,
				    size_t		chunksize )
{
    typedef typename RS_t::symbol_t symbol_t;

    // R-S Decode.  Read up to chunk + parity (or to EOF) symbols of data
    chunk.resize( ( chunksize+rs.NROOTS ) * sizeof symbol_t() );
    inp.read( &chunk.front(), ( chunksize+rs.NROOTS ) * sizeof symbol_t() );
    chunk.resize( inp.gcount() );
    inptotal			       += inp.gcount();

    // Make sure either 0 (EOF at start of input) or an even symbol's worth of data was read.
    if ( chunk.size() == 0 )
	return chunk;
    if ( chunk.size() < ( rs.NROOTS + 1 ) * sizeof symbol_t() || chunk.size() % sizeof symbol_t() )
	throw std::logic_error( std::string() << "Insufficient data for an " << rs << " encoded chunk" );

    // R-S decode (raises std::exception on R-S decode failure), then remove parity
    if ( rs.SYMBOL <= 8 )
    // <= 8-bit symbols.  Correct in-place (leaves parity)
	rs.decode( chunk );
    else {
	// >= 9-bit symbols.  Deserialize to symbols, correct, and re-serialize for output.
	auto			data	= deserialize<symbol_t>( chunk );
	// if errors corrected, serialize corrected symbols back into chunk
	if ( rs.decode( data ))
	    chunk			= serialize<symbol_t>( data );
    }

    // RS_t::decode never discards parity, caller must do so.
    chunk.resize( chunk.size() - rs.NROOTS * sizeof symbol_t() );
    return chunk;
}

int main( int argc, const char **argv )
{
    constexpr size_t		codeword= RSCODEWORD;	// R-S codeword size (always 2^N-1)
    constexpr size_t		parity	= RSPARITY;	// symbols of parity
    size_t			chunksize= RSCHUNK;	// symbols of data chunk (up to codeword-parity)
    typedef ezpwd::RS<codeword,codeword-parity> \
				RS_t;
    RS_t			rs;			// eg; paritysize of 32 ==> RS<255,223>

    size_t			inptotal = 0;
    bool			encoding = true;

    try {
	// Pre-process and discard the initial (executable path) argument, and any '-...' options.
	while ( --argc && **++argv == '-' ) {
	    if ( argv[0][1] == '\0' )			// bare "-"
		break;
	    if ( !strcmp( *argv, "-d" ) || !strcmp( *argv, "--decode" )) {
		encoding		= false;
	    } else if ( !strcmp( *argv, "-e" ) || !strcmp( *argv, "--encode" )) {
		encoding		= true;
	    } else if ( !strcmp( *argv, "-c" ) || !strcmp( *argv, "--chunk" )) {
		if ( --argc <= 0 )
		    throw std::logic_error( std::string() << *argv << " missing value" );
		size_t		end;
		int		val	= std::stoi( *++argv, &end );
		if ( (*argv)[end] || val <= 0 )
		    throw std::logic_error( std::string() << "garbage after chunk size: " << val <<", or bad value: " << *argv );
		chunksize		= val;
	    } else {
		throw std::logic_error(
		    std::string() << "Invalid option: " << *argv << "\n"
		    << "    -e|--encode   -- R-S encode, adding parity (default)" << "\n"
		    << "    -d|--decide   -- R-S decode, correcting errors and removing parity" << "\n"
		    << "    -c|--chunk    -- set data symbol chunks; default: " << chunksize << "\n" );
	    }
	}

	// Set 'inp' to std::cin, or a newly opened file (which will be released at end of scope)
	std::unique_ptr<std::ifstream>
				inpfile;
	if ( argc > 0 and strcmp( argv[0], "-" )) {
	    inpfile.reset( new std::ifstream( argv[1], std::ifstream::binary ));
	}
	std::istream	       &inp( inpfile ? *inpfile : std::cin );

	// Set 'out' to std::cout, or a newly removed and opened file (also release at end of scope)
	std::unique_ptr<std::ofstream>
				outfile;
	if ( argc > 1 and strcmp( argv[1], "-" )) {
	    std::remove( argv[2] );
	    outfile.reset( new std::ofstream( argv[2], std::ifstream::binary ));
	}
	std::ostream	       &out( outfile ? *outfile : std::cout );

	// Read, R-S encode/decode and write chunks
	std::string		chunk;
	while ( inp ) {					// 'til error/eof
	    out << ( encoding ? encode<RS_t> : decode<RS_t> )( rs, inp, inptotal, chunk, chunksize );
	    // std::cerr << "Read: " << inptotal << ", wrote: " << chunk.size() << " bytes." << std::endl;
	}
    } catch ( std::exception &exc ) {
	std::cerr << "Error after " << inptotal << " bytes: " << exc.what() << std::endl;
	return 1;
    }
    return 0;
}
