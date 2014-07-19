
#include <iostream>
#include <ezpwd/rs>
#include <time.h>

#ifndef NROOTS
#define NROOTS 3
#endif

int main()
{
    std::string			ori	= "ba5!l";
    std::cout << "Original: " << ezpwd::hexstr( ori ) << std::endl;

    RS_255( 255-NROOTS )	rs;
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
