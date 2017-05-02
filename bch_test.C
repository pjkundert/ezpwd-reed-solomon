
/*
 * bch_test	-- Iterate all available BCH codecs, comparing their speeds
 */

#include <ezpwd/asserter>

// Djelic GPLv2+ BCH "C" API implementation from Linux kernel.  Requires "standalone" shims for
// user-space to build lib/bch.c implementation; API matches kernel.
#include <ezpwd/bch_base>


// 
// Compare BCH (N, ...) implementations.
// 
//     Iterates over all available correction power T values available for the BCH code of size 2^M-1
// 
//     eg.     255
template <size_t N, size_t B=2>	struct log_{       enum { value = 1 + log_<N/B, B>::value }; };
template <size_t B>		struct log_<1, B>{ enum { value = 0 }; };
template <size_t B>		struct log_<0, B>{ enum { value = 0 }; };
    


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
    size_t			M( log_< N + 1 >::value );
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

    return assert.failures ? 1 : 0;
}
