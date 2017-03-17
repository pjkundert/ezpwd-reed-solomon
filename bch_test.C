
#include <ezpwd/asserter>

// Djelic GPLv2+ BCH implementation from Linux kernel.  Requires "standalone" shims for user-space
// to build lib/bch.c implementation; API matches kernel.
extern "C" {
#include <linux/bch.h>
}

// 
// Compare BCH (N, ...) implementations.
// 
//     Iterates over all available correction power T values available for the BCH code of size 2^M-1
// 
//     eg.     255
template <size_t N, size_t B=2>	struct log_{       enum { value = 1 + log_<N/B, B>::value }; };
template <size_t B>		struct log_<1, B>{ enum { value = 0 }; };
template <size_t B>		struct log_<0, B>{ enum { value = 0 }; };
    

template <size_t N>
double 				compare(
				    ezpwd::asserter    &assert )
{
    
    // ezpwd::log_<N,B> -- compute the log base B of N at compile-time
    // Compute the minimum M required for codeword size N (rounding up to next higher M).
    size_t			M( log_< N + 1 >::value );
    struct bch_control	       *bch;
    size_t			T	= 0;
    while ( !! ( bch = init_bch( M, ++T, 0 ))) {
	// We've obtained a valid BCH control structure for the target BCH M and T.  Present
	// in standard BCH ( <SYMBOLS>-<TRUNCATE>, <PAYLOAD>, <CAPACITY> ) terms
	size_t			K	= N - bch->ecc_bits;
	std::cout << "BCH ("		<< std::setw( 3 ) << (1<<M)-1
		  << " - "		<< std::setw( 3 ) << (1<<M)-1 - N
		  << ", "		<< std::setw( 3 ) << (1<<M)-1 - bch->ecc_bits
		  << ", "		<< std::setw( 3 ) << bch->t
		  << " ); ECC == "	<< std::setw( 3 ) << bch->ecc_bits
		  << " bits parity"
		  << std::endl;
	free_bch( bch );
    }
    return 0.0;
}

int main()
{
    ezpwd::asserter		assert;
    double			avg	= 0;
    int				cnt	= 0;
    avg				       += compare<  31>( assert );	++cnt;
    avg				       += compare<  63>( assert );	++cnt;
    avg				       += compare< 127>( assert );	++cnt;
    avg				       += compare< 255>( assert );	++cnt;
    avg				       += compare< 511>( assert );	++cnt;

    std::cout << std::endl << "BCH(...) EZPWD vs. Djelic's: " << avg/cnt << "% faster (avg.)" << std::endl;

    return assert.failures ? 1 : 0;
}
