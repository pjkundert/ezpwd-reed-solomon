
#include <math.h> // M_PI
#include <cmath>

#include <cstdint>
#include <ezpwd/rs>
#include <ezpwd/output>
#include <ezpwd/ezcod_5>

#if defined( DEBUG )
extern "C" {
#include <rs.h> // Phil Karn's implementation
}
#endif // DEBUG


template < size_t P, size_t L > void
ezcod_5_exercise( const ezpwd::ezcod_5<P,L> &ezc )
{
    // Does location precision scale linearly with the number of symbols provided?  Are errors
    // detected/corrected successfully?

    std::cout
	<< std::endl << std::endl
	<< "Testing EZLOC location coding w/ " << ezc.rscodec.nroots()
	<< " parity; " << ezc.rscodec
	<< " error correction over " << ezc.rscodec.symbol() << "-bit symbols"
	<< std::endl
	<< ezc
	<< std::endl;

    for ( int test = 0; test < 5; ++test ) {
	std::string	manip	= ezc.encode();
	switch ( test ) {
	case 0: std::cout << std::endl << "no errors:" << std::endl;
	    break;
	case 1: std::cout << std::endl << "one erasure: 1/" << P << " parity consumed" << std::endl;
	    manip[8] = '_';
	    break;
	case 2: std::cout << std::endl << "one error: 2/" << P << " parity consumed" << std::endl;
	    manip[1] = ( manip[1] == '0' ? '1' : '0' );
	    break; 
	case 3: std::cout << std::endl << "one error, one erasure; 3/" << P << " parity consumed" << std::endl;
	    manip[8] = '_';
	    manip[1] = ( manip[1] == '0' ? '1' : '0' );
	    break;
	case 4: std::cout << std::endl << "parity capacity overwhelmed" << std::endl;
	    manip[8] = ( manip[8] == '0' ? '1' : '0' );
	    manip[1] = ( manip[1] == '0' ? '1' : '0' );
	    break;
	}
	for ( size_t i = 0; i <= manip.size(); ++i ) {
	    std::string		trunc( manip.begin(), manip.begin() + i );
	    if ( trunc.back() == ' ' )
		continue;
	    trunc.resize( manip.size(), ' ' );
	    ezpwd::ezcod_5<P,L>	code;
	    try {
		int		conf	= code.decode( trunc );
		std::cout
		    << ezpwd::hexstr( trunc ) << " ==> " << code
		    << " (" << std::setw( 3 ) << conf << "%)" << std::endl;
	    } catch ( std::exception &exc ) {
		std::cout
		    << ezpwd::hexstr( trunc ) << " =x> " << exc.what() << std::endl;
	    }
	}
    }
}



int				main()
{
    std::string		abc	= "0123abcz";
    std::string		dec	= abc;
    ezpwd::base32::decode( dec );
    std::cout << ezpwd::hexstr( abc ) << " ==> " << ezpwd::hexstr( dec ) << std::endl;
    std::string		enc	= dec;
    ezpwd::base32::encode( enc );
    std::cout << ezpwd::hexstr( dec ) << " ==> " << ezpwd::hexstr( enc ) << std::endl;

    double		lat	= 53.555556;
    double		lon	= -113.873889;
    ezpwd::ezcod_5<1>	edm1( lat, lon );
    ezcod_5_exercise( edm1 );
    ezpwd::ezcod_5<2>	edm2( lat, lon );
    ezcod_5_exercise( edm2 );
    ezpwd::ezcod_5<3>	edm3( lat, lon );
    ezcod_5_exercise( edm3 );
    ezpwd::ezcod_5<4>	edm4( lat, lon );
    ezcod_5_exercise( edm4 );
    ezpwd::ezcod_5<5>	edm5( lat, lon );
    ezcod_5_exercise( edm5 );


    // Excercise the R-S codecs beyond their correction capability.  This test used to report -'ve
    // error correction positions.  Now, computing -'ve correctly fails the R-S decode, as it
    // indicates that the supplied data's R-S Galois field polynomial solution inferred errors in
    // data we *know* is correct -- the effective block of zero data in the pad (unused) area of the
    // R-S codeword's capacity!

    // Correct encoding w/2 parity:R3U 08M PVT GY
    //                      errors: v      v
    std::string		err2	= "R0U 08M 0VT GY";
    std::string		fix2	= err2;
    ezpwd::base32::decode( fix2 );
    std::vector<int>	pos2;
    int			cor2	= edm2.rscodec.decode( fix2, std::vector<int>(), &pos2  );
    std::string		enc2	= fix2;
    ezpwd::base32::encode( enc2 );
    std::cout
	<< "2 errors (ezpwd::reed_solomon): " << ezpwd::hexstr( err2 )
	<< " --> " << ezpwd::hexstr( enc2 )
	<< "; detected " << cor2 << " errors"
	<< " @" << pos2
	<< std::endl;

#if defined ( DEBUG )
    // Try Phil Karn's R-S codec over RS(31,29), with 2 parity, a capacity of 29 and payload of 9.
    // May compute error positions in "pad" (unused portion), not in supplied data or parity!
    void	       *rs_31_29	= ::init_rs_char( 5, 0x25, 1, 1, 2, 29-9 );
    std::string		fix_31_29	= err2;
    ezpwd::base32::decode( fix_31_29 );
    std::vector<int>	era_31_29;
    era_31_29.resize( 2 );
    int			cor_31_29	= ::decode_rs_char( rs_31_29, (unsigned char *)&fix_31_29.front(),
							    &era_31_29.front(), 0 );
    std::string		enc_31_29	= fix_31_29;
    ezpwd::base32::encode( enc_31_29 );
    era_31_29.resize( std::max( 0, cor_31_29 ));
    std::cout
	<< "2 errors (Phil Karn R-S coded): " << ezpwd::hexstr( err2 )
	<< " --> " << ezpwd::hexstr( enc_31_29 )
	<< "; detected " << cor_31_29 << " errors"
	<< " @" << era_31_29
	<< std::endl;
#endif // DEBUG
    
}
