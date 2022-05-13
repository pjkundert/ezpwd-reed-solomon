
#include <array>
#include <vector>
#include <iostream>
#include <random>

#include <ezpwd/asserter>
#include <ezpwd/output>
#include <ezpwd/bch>

std::minstd_rand		randomizer;
std::uniform_int_distribution<unsigned int>
				random_80( 0, 79 );

#define EZPWD_BCH_FIXED

int main()
{
    // Allocate a BCH codec Galois order 8 (w/ 2^8-1 == 255 bit codeword size), and 2 bits of
    // correction capacity.  This results in a BCH( 255, 239, 2) codec: 255-bit codeword, 239-bit
    // data payload capacity, hence 255-239 == 16 bits of parity.  Define EZPWD_BCH_CLASSIC to use
    // classic Djelic Linux Kernel API.  Otherwise, uses <ezpwd/bch> 'bch<..>' or 'BCH<...>' classes.
    ezpwd::asserter		assert;
    try {
#if defined( EZPWD_BCH_CLASSIC )
        ezpwd::bch_control                 *bch     = ezpwd::init_bch( 8, 2, 0 );
        if ( ! bch )
    	throw std::logic_error( "Failed to allocate valid BCH codec" );
        ezpwd::bch_control                 &bch_codec( *bch );	// By Galois order, Correction capacity
#else
        //ezpwd::bch_base		bch_codec( 8, 2 );	// By Galois order, Correction capacity, flexibly
        ezpwd::BCH<255,239,2>		bch_codec;	// By Codeword, Payload and Correction capacities, exactly
#endif

#if defined( EZPWD_BCH_FIXED )
        typedef std::array<uint8_t,10>	codeword_t;		// Fixed (parity initialized to 0)
#else
        typedef std::vector<uint8_t>	codeword_t;		// Variable (parity added by encode)
#endif

	codeword_t				codeword= {
	    0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF	// 8 data
	};							// 2 ECC parity

#if defined( EZPWD_BCH_CLASSIC )
	// A BCH codeword's ECC must be zero-initialized for classic Djelic API, and
	// must be added to variable containers
#  if ! defined( EZPWD_BCH_FIXED )
	codeword.resize( 10 );
#  endif
	codeword[8] 				= 0;
	codeword[9] 				= 0;
	ezpwd::encode_bch( &bch_codec, &codeword[0], 8, &codeword[8] );
#else
	// A codeword in a fixed container will not be resized by API; assumes ECC on end. Will
	// initialize to 0.  A variable container will have parity appended by the API.
	bch_codec.encode( codeword );
#endif

	// Place random errors in the codeword and correct, up to the capacity of the BCH codec.
	for ( size_t t = 0; t < 5; ++t ) {
	    for ( size_t e = 0
#if defined( EZPWD_BCH_CLASSIC )
		      ; e < bch_codec.t
#else
		      ; e < bch_codec.t()
#endif
		      ; ++e ) {
		codeword_t		corrupted( codeword );
		// Randomly corrupt from 0 to bch->t bits
		for ( size_t be = 0; be < e; ++be ) {
		    unsigned int	bl	= random_80( randomizer );
		    corrupted[bl/8]	       ^= uint8_t( 1 ) << ( bl % 8 );
		}
		codeword_t		corrected( corrupted );
#if defined( EZPWD_BCH_CLASSIC )
		int corrections			= ezpwd::correct_bch( &bch_codec, &corrected[0], 8, &corrected[8] );
#else
		int corrections			= bch_codec.decode( corrected );
#endif

		if ( assert.ISEQUAL( corrections, int( e ),
				     std::string( "Failed decode of " ) << bch_codec
				     << " codeword w/ " << e << " bit errrors"
				     )) {
		    std::cout << assert << std::endl;
		    continue;
		}

		// Success; If differences were found, they better be in the parity data!
		for ( size_t i = 0; i < 8; ++i )
		    if ( assert.ISEQUAL( corrected[i], codeword[i], 
					 std::string( "Failed recovery of " ) << bch_codec
					 << " codeword w/ " << e << " bit errors" ))
			std::cout << assert << std::endl;
	    }
#if ! defined( EZPWD_BCH_CLASSIC )
	    // Try the inline versions (only available in C++ API)
	    std::string			raw( "abc" );
	    std::string			enc	= bch_codec.encoded( raw );
	    std::string			err	= enc;
	    err[0]			       ^= 0x40;
	    err[2]			       ^= 0x08;
	    std::string			dec	= bch_codec.decoded( err );
	    if ( assert.ISEQUAL( dec.substr( 0, 3 ), raw,
				 std::string( "decoded/encoded of " )	<< bch_codec
				 << " codeword failed, encoded '0x"	<< ezpwd::hexstr( raw )
				 << "' to '0x"				<< ezpwd::hexstr( enc )
				 << "', decoded '0x"			<< ezpwd::hexstr( err )
				 << "' to '0x"				<< ezpwd::hexstr( dec )
				 << "'" ))
		std::cout << assert << std::endl;
#endif
	}
	std::cout
	    << bch_codec << ": ";
    } catch( std::exception &exc ) {
	assert.FAILURE( "Exception", exc.what() );
	std::cout << assert << std::endl;
    }

    if ( assert.failures )
	std::cout
	    << assert.failures << " tests failed." << std::endl;
    else
	std::cout
	    << "All tests passed." << std::endl;
    return assert.failures ? 1 : 0;
}
