
/*
 * bch_test	-- Iterate all available BCH codecs, comparing their speeds
 */

#include <vector>
#include <map>
#include <set>
#include <iostream>
#include <random>

#include <ezpwd/asserter>

// Djelic GPLv2+ BCH "C" API implementation from Linux kernel.  Requires "standalone" shims for
// user-space to build lib/bch.c implementation; API matches kernel.
#include <ezpwd/bch>


//
// compare< N> -- compare BCH implementations for N-bit codewords
// 
//     Allocates all BCH codecs for N-bit (eg. 31, ..., 511) codewords, supporting
// various bit-error correction capacities T.
// 
//     Computes the Galois order M, which fits the desired codeword size N, in bits.  Then, iterates
// over increasing desired bit correction capacities, until failure.  Logs the standard BCH
// specification, in bits: BCH( N, N-ECC, T ), where N == total codeword size, N-ECC == payload
// size, T == correction capacity.
// 
template <size_t N>
double 				compare(
				    ezpwd::asserter    &assert )
{
    
    // ezpwd::log_<N,B> -- compute the log base B of N at compile-time
    // Compute the minimum M required for codeword size N (rounding up to next higher M).
    size_t			M( ezpwd::log_< N + 1 >::value );
    struct ezpwd::bch_control  *bch;
    size_t			T	= 0;
    while ( !! ( bch = ezpwd::init_bch( M, ++T, 0 ))) {
	// We've obtained a valid BCH control structure for the target BCH M and T.  Present
	// in standard BCH ( <SYMBOLS>, <PAYLOAD>, <CAPACITY> ) terms
	std::cout << *bch // (see c++/ezpwd/bch_base for formatter)
		  << "; ECC = "	<< std::setw( 3 ) << bch->ecc_bits
		  << " / "	<< std::setw( 3 ) << bch->ecc_bytes
		  << std::endl;
	ezpwd::free_bch( bch );
    }
    (void)assert; // no tests, yet.
    return 0.0;
}

std::minstd_rand		randomizer;
std::uniform_int_distribution<uint8_t>
				random_byte( 0, 255 );

int main()
{
    ezpwd::asserter		assert;
    double			avg	= 0;
    int				cnt	= 0;
    std::cout << "BCH Codecs Available (in bits)" << std::endl;
    std::cout << "  size of" << std::endl;
    std::cout << "  codeword" << std::endl;
    std::cout << "       |  data" << std::endl;
    std::cout << "       |  payload" << std::endl;
    std::cout << "       |    |    bit error" << std::endl;
    std::cout << "       |    |    capacity" << std::endl;
    std::cout << "       |    |    |           parity" << std::endl;
    std::cout << "       |    |    |           bits  bytes" << std::endl;
    std::cout << "       |    |    |            |     |" << std::endl;
    avg				       += compare<  31>( assert );	++cnt;
    avg				       += compare<  63>( assert );	++cnt;
    avg				       += compare< 127>( assert );	++cnt;
    avg				       += compare< 255>( assert );	++cnt;
    avg				       += compare< 511>( assert );	++cnt;

    std::cout << std::endl << "BCH(...) EZPWD vs. Djelic's: " << avg/cnt << "% faster (avg.)" << std::endl;


    // Evaluate the following BCH codec, to determine its effectiveness at detecting random bit
    // errors.
    constexpr size_t		sym	= 255;
    constexpr size_t		pay	= 239;		// max. net payload
    constexpr size_t		par	= sym - pay;
    constexpr size_t		cap	= 2;
 // constexpr size_t		paymin	= cap * 2;

    // Due to there being no way (presently) to specify a number of payload bits
    // that is a fraction of the (bit-packed) container supplied, there is no way to
    // use the ezpwd::bch API to supply the full 239-bit payload; the next lower
    // multiple of an 8-bit byte (232) is the maximum possible.
    std::vector<size_t>		len	{
	8, 16, 32, 50, 64, 80, 100, 128, 200, 232, pay
    };
    size_t			run	= 1000000;
    ezpwd::BCH<sym,pay,cap>	bch;

    std::cout
	<< std::setw( 8 * ( bch.t() * 2 + 2 )) << std::left << "Corrections"
	<< " | " << std::right << "Description"
	<< std::endl;
    
    for ( int i = -1; i <= int( bch.t() ) * 2; ++i )
	std::cout
	    << std::setw( 8 ) << i;
    std::cout
	<< " | "
	<< std::endl
	<< std::string( 8 * ( bch.t() * 2 + 2 ), '-' )
	<< " | "
	<< std::endl;
    for ( size_t l : len ) {
	std::vector<uint8_t>	payload( ( l + 7 ) / 8 );
	for ( uint8_t &v : payload )
	    v				= random_byte( randomizer );
	std::uniform_int_distribution<size_t>
			random_msgbit( 0, par + l - 1 );	// [0,parity + payload)
	int		e_max	= std::min( l, bch.t() * 4 );
	for ( int e			= 0
		  ; e <= e_max + 1
		  ; ++e ) { // > e_max --> random date
	    std::map<int,size_t> errfix;			// bit errors fixed:  [-1,paymin] --> <count>
	    for ( size_t test = 0; test < run; ++test ) {
		// For each test, change a byte and re-generate parity.  We want to get a valid
		// distribution of error correction capacity; not the capacity of any specific
		// codeword.  Always execute on test == 0!
		payload[test % payload.size()]
		    			= random_byte( randomizer );
		std::vector<uint8_t>
		    		parity( ( par + 7 ) / 8 );
		for ( uint8_t &v : parity )
		    v			= random_byte( randomizer );

		// Test for correction of all random data.  May change parity!
		std::vector<uint8_t>
		    		errload( payload );

		int		fixed	= -1;
		if ( e <= e_max ) {
		    // Test for correction of 'e' bit errors; fills in parity with BCH ECC data
		    bch.encode( payload, parity );
		    std::set<int> used;
		    for ( int en = 0; en < e; ++en ) {
			while ( true ) {
			    size_t eb = random_msgbit( randomizer );
			    if ( used.find( eb ) != used.end() )
				continue;
			    used.insert( eb );
			    if ( eb < l ) {
				// Payload corruption
				errload[eb/8]  ^= uint8_t( 1 ) << ( eb % 8 );
			    } else {
				// Parity corruption
				parity[(eb-l)/8] ^= uint8_t( 1 ) << ( (eb-l) % 8 );
			    }
			    break;
			}
		    }
		}

		try { fixed		= bch.decode( errload, parity ); } catch ( ... ) { fixed = -1; }
		++errfix[fixed >= 0 ? fixed : -1];
	    }
	    for ( int i = -1; i <= int( bch.t() * 2 ); ++i )
		std::cout
		    << std::setw( 8 ) << errfix[i];

	    std::cout
		<< " | BCH("		<< std::setw( 3 ) << sym
		<< ","			<< std::setw( 3 ) << pay
		<< ","			<< std::setw( 3 ) << cap
		<< ") w/ "		<< std::setw( 3 ) << l
		<< " payload, ";
	    if ( e > e_max )
		std::cout
		    << "random data/parity"
		    << std::endl
		    << std::endl;
	    else
		std::cout
		    << e << " bit errors"
		    << std::endl;
	}
    }
    
    return assert.failures ? 1 : 0;
}
