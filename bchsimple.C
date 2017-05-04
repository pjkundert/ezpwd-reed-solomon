
#include <array>
#include <iostream>
#include <random>

#include <ezpwd/bch>

std::minstd_rand		randomizer;
std::uniform_int_distribution<unsigned int>
				random_80( 0, 79 );

int main()
{
    // Allocate a BCH codec w/ 2^8-1 == 255 bit codeword size, and 2 bits of correction capacity.
    // This results in a BCH( 255, 239, 2) codec: 255-bit codeword, 239-bit data payload capacity,
    // hence 255-239 == 16 bits of parity.

    //ezpwd::bch_control                 *bch     = ezpwd::init_bch( 8, 2, 0 );
    //ezpwd::bch_base			bch_255_239_2( 8, 2 );
    ezpwd::BCH<255,239,2>		bch_255_239_2;

    std::array<uint8_t,10>		codeword= {
	0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,		// data
	0, 0 };							// parity, initialized to 0
    //ezpwd::encode_bch( bch, &codeword[0], 8, &codeword[8] );
    bch_255_239_2.encode( codeword );

    // Place random errors in the codeword and correct, up to the capacity of the BCH codec.
    size_t				failures= 0;
    for ( size_t t = 0; t < 5; ++t ) {
	//for ( size_t e = 0; e < bch->t; ++e ) {
	for ( size_t e = 0; e < bch_255_239_2.t(); ++e ) {
	    std::array<uint8_t,10>	corrupted( codeword );
	    // Randomly corrupt from 0 to bch->t bits
	    for ( size_t be = 0; be < e; ++be ) {
		unsigned int		bl	= random_80( randomizer );
		corrupted[bl/8]		       ^= uint8_t( 1 ) << ( bl % 8 );
	    }
	    std::array<uint8_t,10>	corrected( corrupted );

	    //int corrections			= correct_bch( bch, &corrected[0], 8, &corrected[8] );
	    int corrections			= bch_255_239_2.decode( corrected );

	    if ( corrections != int( e )) {
		//std::cout << "Failed decode of " << *bch << " codeword" << std::endl;
		std::cout << "Failed decode of " << bch_255_239_2 << " codeword" << std::endl;
		++failures;
	    }
	    // If differences were found, they better be in the parity data!
	    for ( size_t i = 0; i < 8; ++i ) {
		if ( corrected[i] != codeword[i] ) {
		    //std::cout << "Failed recovery of " << *bch
		    std::cout << "Failed recovery of " << bch_255_239_2
			      << " codeword w/ " << e << " bit errors "
			      << std::endl;
		    ++failures;
		}
	    }
	}
    }
    if ( failures )
	std::cout << bch_255_239_2 // *bch
		  << ": " << failures << " tests failed." << std::endl;
    else
	std::cout << bch_255_239_2 // *bch
		  << ": All tests passed." << std::endl;
    return failures ? 1 : 0;
}
