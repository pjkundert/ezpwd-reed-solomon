
/*
 * bch_itron	-- Implement the Itron BCH coding used for some ERT digital "smart" meters
 * 
 *     The Itron ERT/Collector uses a "shortened" (via truncation) BCH (255, 239, T=2) code, with 16
 * bits of parity data to validate and/or error-correct its data messages.
 * 
 */

#include <ezpwd/asserter>

// Djelic GPLv2+ BCH implementation from Linux kernel.  Requires "standalone" shims for user-space
// to build lib/bch.c implementation; API matches kernel.
extern "C" {
#include <linux/bch.h>
}



int main()
{
    ezpwd::asserter		assert;

    // Ensure we can get the require BCH (255, 239, T=2) codec we require.  There's no way to
    // confirm that the generator polynomial is what we expect, as it is discarded by init_bch after
    // generating the internal tables.  However, I've confirmed that it is 0b10110111101100011 ==
    // 0o267543 == 0x16f63.  Strangely, when generating the BCH code using a 16-bit CRC generator,
    // other projects (namely rtl-amr) use the polynomial 0x6f63...
    struct bch_control	       *bch	= init_bch( 8, 2, 0 );
    assert.ISEQUAL( bch->m,		  8U );
    assert.ISEQUAL( bch->n,		255U );
    assert.ISEQUAL( bch->t,		  2U );
    assert.ISEQUAL( bch->ecc_bits,	 16U );

    
    
    return assert.failures ? 1 : 0;
}
