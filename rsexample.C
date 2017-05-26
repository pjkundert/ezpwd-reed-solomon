// 
// rsexample.C -- Illustrate the use of the R-S codec and ezpwd::corrector APIs
// 
//     There are cases where we may have to guess how many symbols have R-S symbols been provided.
// Such is the case when we know how many R-S parity symbols are to be provided, but it is unknown
// how long the protected non-parity data is, nor how many of the parity symbols have been supplied.
// 
//     We can guess, and return an estimated likelihood that the supplied data is correct.  This is
// what ezpwd::corrector does.  Instantiate it with the known number of R-S parity symbols, and it
// will evaluate the supplied codeword, performing R-S decodes with varying guesses of data and
// parity symbols, returning the most likely candidate data, with an estimate of its probability of
// correctness.
// 
#include <time.h>

#include <iostream>

#include <ezpwd/rs>
#include <ezpwd/corrector>
#include <ezpwd/output>

#include <ezpwd/definitions>	// must be included in one C++ compilation unit

#ifndef NROOTS
#define NROOTS 3
#endif

int main()
{
    std::string			ori	= "ba5!l";
    std::cout << "Original: " << ezpwd::hexstr( ori ) << std::endl;

    ezpwd::RS<255,255-NROOTS>	rs;
    for ( size_t errpos = 0; errpos < ori.size() + NROOTS + 1; ++errpos ) {
	std::cout << "Raw Reed-Solomon codec:" << std::endl;
	std::string		enc	= ori;
	rs.encode( enc );
	std::cout << "Encoded:  " << ezpwd::hexstr( enc ) << std::endl;
	std::string		err	= enc;
	if ( errpos < err.size() ) {
	    err[errpos]		       ^= 0x15; // simulated mistyped character
	    std::cout << "Mistyped: " << ezpwd::hexstr( err ) << std::endl;
	} else
	    std::cout << "Mistyped: (no)" << std::endl;

	std::string		dec	= err;
	rs.decode( dec );
	std::cout << "Decoded:  " << ezpwd::hexstr( dec ) << std::endl;

	std::cout << "Statistical corrector:" << std::endl;
	enc					= ori;
	ezpwd::corrector<NROOTS>::encode( enc );
	std::cout << "Encoded:  " << ezpwd::hexstr( enc ) << std::endl;
	err				= enc;
	if ( errpos < err.size() ) {
	    err[errpos]		       ^= 0x15; // simulated mistyped character
	    std::cout << "Mistyped: " << ezpwd::hexstr( err ) << std::endl;
	} else
	    std::cout << "Mistyped: (no)" << std::endl;
	for ( size_t len = 1; len <= err.size(); ++len ) {
	    std::string		sub	= err.substr( 0, len );
	    std::string		cor	= sub;
	    int			confid	= ezpwd::corrector<NROOTS>::decode( cor );
	    std::string		dsp	= cor;

	    ezpwd::corrector<NROOTS>::encode( dsp );
	    if ( confid > 0 ) {
		//dsp.insert( 0, "\xE2\x98\x91 " );  // ballot check
		dsp.insert( 0, "\xE2\x9C\x94 " );  // heavy check
	    } else {
		//dsp.insert( 0, "\xE2\x98\x90 " ); // ballot x
		dsp.insert( 0, "\xE2\x9C\x98 " ); // heavy X
	    }
	    dsp.insert( dsp.size() - 3, " (" ) += ')';
	    std::cout
		<< "Correct:  " << ezpwd::hexstr( sub ) 
		<< std::setw( enc.size() > sub.size()
			      ? 2 * ( enc.size() - sub.size() )
			      : 0 ) << ""
		<< " ==> " << ezpwd::hexstr( cor )
		<< std::setw( enc.size() > cor.size()
			      ? 2 * ( enc.size() - cor.size() )
			      : 0 ) << ""
		<< " w/ confidence " << std::setw( 3 )  << confid << "%"
		<< " shows: " << dsp
		<< std::endl;
	}
    }

}
