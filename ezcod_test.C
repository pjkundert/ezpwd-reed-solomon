
#include <math.h> // M_PI
#include <cmath>
#include <random>
#include <iostream>
#include <sstream>

#include <cstdint>
#include <map>
#include <list>
#include <vector>
#include <memory>

#include <ezpwd/rs>
#include <ezpwd/output>
#include <ezpwd/asserter>
#include <ezpwd/ezcod>
#include <ezpwd/serialize>

#include "ezcod.h"		// C API declarations

#if defined( DEBUG ) && DEBUG > 2
extern "C" {
#include <rs.h> // Phil Karn's implementation
}
#endif // DEBUG


template < unsigned P, unsigned L > void
ezcod_exercise( const ezpwd::ezcod<P,L> &ezc )
{
    // Does location precision scale linearly with the number of symbols provided?  Are errors
    // detected/corrected successfully?

    std::cout
	<< std::endl << std::endl
	<< "Testing ezcod<"
	<< int(ezc.symbols().first) << "," << int(ezc.symbols().second)
	<< "> location coding"
#if defined( DEBUG )
	<< " w/ " << ezc.rscodec.nroots()
	<< " parity; " << ezc.rscodec
	<< " error correction over " << ezc.rscodec.symbol() << "-bit symbols"
#endif
	<< std::endl
	<< ezc
	<< std::endl
	<< std::endl
	<< std::endl;


    for ( int test = 0; test < 5; ++test ) {
	std::string	manip	= ezc.encode();
	switch ( test ) {
	case 0: default: std::cout << std::endl << "no errors:" << std::endl;
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
	for ( unsigned i = 0; i <= manip.size(); ++i ) {
	    std::string		trunc( manip.begin(), manip.begin() + i );
	    if ( trunc.back() == ' ' )
		continue;
	    trunc.resize( manip.size(), ' ' );
	    ezpwd::ezcod<P,L>	code;
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

// Test EZCOD accuracy in the specified number of data symbols (default: 9)

int				main( int argc, char **argv )
{
    ezpwd::asserter		assert;
    int				symbols	= 9;
    if ( argc > 1 )
	std::istringstream( argv[1] ) >> symbols;

    // Tests of basic facilities
    char			buf[16]	= { 0 };
    ezpwd::streambuf_to_buffer	sbf( buf, sizeof buf );
    std::ostream 		obf( &sbf );

    obf << "String" << ' ' << 123 << std::endl;
    assert.ISEQUAL( strlen( buf ), size_t( 11 ));
    assert.ISEQUAL( strcmp( buf, "String 123\n" ), 0 );
    obf << "Too Long!";
    assert.ISEQUAL( strcmp( buf, "String 123\nToo " ), 0 );


    std::string			abc	= "0123abcz";
    std::string			dec	= abc;
    ezpwd::serialize::base32::decode( dec );
    // std::cout << ezpwd::hexstr( abc ) << " ==> " << ezpwd::hexstr( dec ) << std::endl;
    std::string			enc	= dec;
    ezpwd::serialize::base32::encode( enc );
    // std::cout << ezpwd::hexstr( dec ) << " ==> " << ezpwd::hexstr( enc ) << std::endl;
    if ( assert.ISEQUAL( enc, std::string( "0123ABC2" )))
	std::cout << assert << std::endl;

    // Ensure that EZCOD codec with various parity sizes do not erroneously accept EZCODs of
    // different parity sizes.  For example, an ezcod<1,9> codec could parse an EZCOD with any
    // number of location precision symbols, but only with 1 parity symbol -- it doesn't have an R-S
    // codec capable of validating the R-S codeword containing (say) 2 or 3 parity symbols.
    // However, an R-S codeword with X symbols and Y parity is also an R-S codeword with X-1 symbols
    // and Y+1 parity!  So, we can validate the codeword with TOO MANY parity symbols for the R-S
    // codec, by treating the leading excess parity symbols as part of the data.  Of course, we'll
    // only be able to recover from the (lower) error/erasure capacity of our smaller R-S codec.
    ezpwd::ezcod<1,9>		ec3m_10(   53.555518, -113.873530 ); // RS<31,30> ==> 1 parity
    ezpwd::ezcod<3,12>		ec20mm_15( 53.555518, -113.873530 ); // RS<31,28> ==> 3 parity
    ezpwd::ezcod<5,12>		ec20mm_17( 53.555518, -113.873530 ); // RS<31,26> ==> 5 parity
    if ( assert.ISEQUAL( ec3m_10.encode(),   std::string( "R3U 08M PXT.5" )))
	std::cout << assert << std::endl;
    if ( assert.ISEQUAL( ec20mm_15.encode(), std::string( "R3U 08M PXT 31N.L2H" )))
	std::cout << assert << std::endl;
    if ( assert.ISEQUAL( ec20mm_17.encode(), std::string( "R3U 08M PXT 31N.71K3E" )))
	std::cout << assert << std::endl;
    try {
	ezpwd::ezcod<3,12>	dc20mm_15_2;
	int 			validity = dc20mm_15_2.decode( "R3U 08M PXT 3xN.71K3E" );
	if ( assert.ISEQUAL( validity, 34 ))
	    std::cout << assert << std::endl;
	//std::cout << dc20mm_15_2 << ": After decode of 5-parity EZCOD by 3-parity codec" << std::endl;
	validity 			= dc20mm_15_2.decode( "R3U 08M PXT 3x_.71K3E" );
	if ( assert.ISEQUAL( validity, 0 )) // all R-S parity of RS<31,28> ==> 3 parity codec consumed!
	    std::cout << assert << std::endl;
	//std::cout << dc20mm_15_2 << ": After decode of 5-parity EZCOD by 3-parity codec" << std::endl;
	// OK, try a higher-parity codeword in a lower-parity R-S codec...
	std::string		ec12_5( "R3U 08M PXT 31N-71K3E" );
	ezpwd::serialize::base32::decode( ec12_5 );
	ezpwd::RS<31,26>	rs_31_26;
	if ( assert.ISEQUAL( rs_31_26.decode( ec12_5 ), 0 )) // No corrections; should be valid 12+5 codeword
	    std::cout << assert << std::endl;
	ezpwd::RS<31,28>	rs_31_28;
	if ( assert.ISEQUAL( rs_31_28.decode( ec12_5 ), 0 )) // No corrections; should ALSO be  14+3 codeword!
	    std::cout << assert << std::endl;
    } catch ( std::exception &exc ) {
	std::cout << "R-S aliasing tests failed: " << exc.what() << std::endl;
    }

    double			lat	=   53.555556;
    double			lon	= -113.873889;
    std::map<std::pair<unsigned,unsigned>,std::string>
				str	= {
	{{1, 3},"R3U.9"},
	{{2, 3},"R3U.WD"},
	{{3, 3},"R3U.09K"},
	{{1, 4},"R3U0.J"},
	{{2, 4},"R3U0.9K"},
	{{3, 4},"R3U0.9K0"},
	{{1, 5},"R3U08.H"},
	{{1, 6},"R3U 08M.8"},
	{{1, 7},"R3U 08MP.U"},
	{{1, 8},"R3U 08MPV.E"},
	{{1, 9},"R3U 08M PVT.D"},
	{{1,10},"R3U 08M PVTQ.F"},
	{{1,11},"R3U 08M PVTQJ.Y"},
	{{1,12},"R3U 08M PVT QJQ.E"}};
    std::map<std::pair<unsigned,unsigned>,double>
   				acc	= {{{1, 3}, 90611.067453 },
					   {{2, 3}, 90611.067453 },
					   {{3, 3}, 90611.067453 },
					   {{1, 4}, 20387.698592 },
					   {{2, 4}, 20387.698592 },
					   {{3, 4}, 20387.698592 },
					   {{1, 5},  2841.975017 },
					   {{1, 6},   637.189335 },
					   {{1, 7},    88.807584 },
					   {{1, 8},    19.912068 },
					   {{1, 9},     2.775226 },
					   {{1,10},     0.622252 },
					   {{1,11},     0.086726 },
					   {{1,12},     0.019445 }};

    {
	ezpwd::ezcod_base	       *ezc[] = {
	    new ezpwd::ezcod<1, 3>( lat, lon ),
	    new ezpwd::ezcod<2, 3>( lat, lon ),
	    new ezpwd::ezcod<3, 3>( lat, lon ),
	    new ezpwd::ezcod<1, 4>( lat, lon ),
	    new ezpwd::ezcod<2, 4>( lat, lon ),
	    new ezpwd::ezcod<3, 4>( lat, lon ),
	    new ezpwd::ezcod<1, 5>( lat, lon ),
	    new ezpwd::ezcod<1, 6>( lat, lon ),
	    new ezpwd::ezcod<1, 7>( lat, lon ),
	    new ezpwd::ezcod<1, 8>( lat, lon ),
	    new ezpwd::ezcod<1, 9>( lat, lon ),
	    new ezpwd::ezcod<1,10>( lat, lon ),
	    new ezpwd::ezcod<1,11>( lat, lon ),
	    new ezpwd::ezcod<1,11>( lat, lon ),
	    new ezpwd::ezcod<1,12>( lat, lon )
	};

	// Try all the practical variants of Location and Parity
	for ( auto e : ezc ) {
	    if ( assert.ISEQUAL( e->encode(), str[e->symbols()] ))
		std::cout << "On " << *e << ": " << assert;
	    try {
		e->decode( str[e->symbols()] );
		if ( assert.ISNEAR( e->accuracy, acc[e->symbols()], 1e-4 ))
		    std::cout << "On " << *e << ": " << assert;
	    } catch ( std::exception &exc ) {
		assert.ISTRUE( false, exc.what() );
		std::cout << "On " << *e << ": " << assert;
	    }
	    std::cout << *e << std::endl;
	    delete e;
	}
    }

    ezpwd::ezcod<1>		edm1( lat, lon );
    ezcod_exercise( edm1 );
    ezpwd::ezcod<2>		edm2( lat, lon );
    ezcod_exercise( edm2 );
    ezpwd::ezcod<3>		edm3( lat, lon );
    ezcod_exercise( edm3 );
    ezpwd::ezcod<4>		edm4( lat, lon );
    ezcod_exercise( edm4 );
    ezpwd::ezcod<5>		edm5( lat, lon );
    ezcod_exercise( edm5 );


    // Ensure we can encode and decode any valid number of position symbols, w/ any
    // ezpwd::ezcod<PARITY,...> codec, with any number of remaining parity symbols, so long as we
    // retain the position-parity separator.
    edm5.latitude		= 53.555518;
    edm5.longitude		= -113.873530;
    std::string		e5p3	= edm5.encode( 3 );
    if ( assert.ISEQUAL( e5p3, std::string( "R3U.XVVHH" )))
	std::cout << assert << std::endl;
    for ( ; e5p3.back() != 'U'; e5p3.resize( e5p3.size() - 1 )) {
	int		cnf	= edm5.decode( e5p3 );
	//std::cout << "Got: " << edm5 << " from: " << e5p3 << " w/ confidence " << cnf <<std::endl;
	if ( assert.ISTRUE( e5p3.back() == '.' ? cnf == 0 : cnf >= 0 ))
	    std::cout << "For " << e5p3 << ": " << assert << std::endl;
    }

    edm5.latitude		= 53.555518;
    edm5.longitude		= -113.873530;
    std::string		e5p12	= edm5.encode( 12 );
    if ( assert.ISEQUAL( e5p12, std::string( "R3U 08M PXT 31N.71K3E" )))
	std::cout << assert << std::endl;
    for ( ; e5p12.back() != 'N'; e5p12.resize( e5p12.size() - 1 )) {
	int		cnf	= edm5.decode( e5p12 );
	//std::cout << "Got: " << edm5 << " from: " << e5p12 << " w/ confidence " << cnf <<std::endl;
	if ( assert.ISTRUE( e5p12.back() == '.' ? cnf == 0 : cnf >= 0 ))
	    std::cout << "For " << e5p12 << ": " << assert << std::endl;
    }

    // Ensure that various chunk sizes work
    std::map<int,std::string>	chunks = {
	{ 0, "R3U08MPXT31N.71K3E" },
	{ 1, "R 3 U 0 8 M P X T 3 1 N.71K3E" },
	{ 2, "R3 U0 8M PX T3 1N.71K3E" },
	{ 3, "R3U 08M PXT 31N.71K3E" },
	{ 4, "R3U0 8MPX T31N.71K3E" },
	{ 5, "R3U08 MPXT31N.71K3E" },
	{ 6, "R3U08M PXT31N.71K3E" },
	{ 7, "R3U08MPXT31N.71K3E" },
	{ 8, "R3U08MPXT31N.71K3E" },
	{ 9, "R3U08MPXT31N.71K3E" },
	{10, "R3U08MPXT31N.71K3E" },
	{11, "R3U08MPXT31N.71K3E" },
	{12, "R3U08MPXT31N.71K3E" },
	{13, "R3U08MPXT31N.71K3E" },
	{14, "R3U08MPXT31N.71K3E" },
    };

    for ( int c = 0; c < 15; ++c ) {
	edm5.chunk		= c;
	if ( assert.ISEQUAL( edm5.encode( 12 ), chunks[c] ))
	    std::cout << "For " << edm5 << " w/ chunk == " << c << ": " << assert << std::endl;
    }

   
    // Excercise the R-S codecs beyond their correction capability.  This test used to report -'ve
    // error correction positions.  Now, computing -'ve correctly fails the R-S decode, as it
    // indicates that the supplied data's R-S Galois field polynomial solution inferred errors in
    // data we *know* is correct -- the effective block of zero data in the pad (unused) area of the
    // R-S codeword's capacity!
    // Correct encoding w/2 parity:        R3U 08M PVT GY
    //                              errors: v      v
    std::string			err2	= "R0U 08M 0VT GY";
#if defined( DEBUG )
    std::string			fix2	= err2;
    ezpwd::serialize::base32::decode( fix2 );
    std::vector<int>		pos2;
    int				cor2	= edm2.rscodec.decode( fix2, std::vector<int>(), &pos2  );
    std::string			enc2	= fix2;
    ezpwd::serialize::base32::encode( enc2 );
    std::cout
	<< "2 errors (ezpwd::reed_solomon): " << ezpwd::hexstr( err2 )
	<< " --> " << ezpwd::hexstr( enc2 )
	<< "; detected " << cor2 << " errors"
	<< " @" << pos2
	<< std::endl;
#endif // DEBUG

#if defined( DEBUG ) && DEBUG > 2
    // Try Phil Karn's R-S codec over RS(31,29), with 2 parity, a capacity of 29 and payload of 9.
    // May compute error positions in "pad" (unused portion), not in supplied data or parity!
    void	       	       *rs_31_29= ::init_rs_char( 5, 0x25, 1, 1, 2, 29-9 );
    std::string			fix_31_29= err2;
    ezpwd::serialize::base32::decode( fix_31_29 );
    std::vector<int>		era_31_29;
    era_31_29.resize( 2 );
    int				cor_31_29= ::decode_rs_char( rs_31_29, (unsigned char *)&fix_31_29.front(),
							    &era_31_29.front(), 0 );
    std::string			enc_31_29= fix_31_29;
    ezpwd::serialize::base32::encode( enc_31_29 );
    era_31_29.resize( std::max( 0, cor_31_29 ));
    std::cout
	<< "2 errors (Phil Karn R-S coded): " << ezpwd::hexstr( err2 )
	<< " --> " << ezpwd::hexstr( enc_31_29 )
	<< "; detected " << cor_31_29 << " errors"
	<< " @" << era_31_29
	<< std::endl;
#endif // DEBUG

    // Test the C EZCOD API beyond correction capacity.  Should return -'ve value, and put an error
    // description in the supplied buffer.
    {
	char			dec[1024];
	*std::copy( err2.begin(), err2.end(), dec ) = 0; // copy into dec and NUL-terminate
	double			lat, lon, acc;
	int			res	= ezcod_3_10_decode( dec, sizeof dec, &lat, &lon, &acc );
	if ( assert.ISTRUE( res < 0, "ezcod_3_10_decode should have failed due to error overload" ))
	    std::cout << assert << std::endl;
	std::cout << "2 errors (ezcod_3_10_decode): " << dec << std::endl;
    }
    

    // Test the actual precision of ezcod for various lat/lon positions.  The returned value should
    // always be within the given accuracy radius.
    std::minstd_rand		rnd_gen( (unsigned int)time( 0 ));
    auto			rnd_dbl	= std::uniform_real_distribution<double>( 0.0, 1.0 );

    
    typedef std::map<int, std::pair<int, double>> // <degrees> --> (<count>,<average>)
		     		deg_err_t;
    deg_err_t			lat_err;  // average absolute error; should tend toward avg. error bar
    deg_err_t			lon_err;
    deg_err_t			lat_dif;  // average signed difference; should tend toward 0
    deg_err_t			lon_dif;
    deg_err_t			lat_acc;  // average reported accuracy
    deg_err_t			lon_acc;
    deg_err_t			lat_all;  // linear combined error by latitude, longitude
    deg_err_t			lon_all;
    deg_err_t			lat_max;  // maximum error enountered by latitude, longitude
    deg_err_t			lon_max;
    std::cout << std::setprecision( 8 );
    for ( int i = 0; i < 500000; ++i ) {
	double			lat	= 90;
	double			lon	= 180;
	switch ( i ) {
	case 0:
	    break;
	case 1:
	    lat				= -lat;
	    break;
	case 2:
	    lon				= -lon;
	    break;
	case 3:
	    lat				= -lat;
	    lon				= -lon;
	    break;
	default:
	    lat				= 180 * rnd_dbl( rnd_gen ) - 90;
	    lon				= 360 * rnd_dbl( rnd_gen ) - 180;
	    break;
	}
	int			lat_i	= int( lat + ( lat > 0 ? .5 : -.5 ));
	int			lon_i	= int( lon + ( lon > 0 ? .5 : -.5 ));

	// Get EZCOD using C API for chuckles; gotta love NUL terminated strings...
	std::string		cod;
	cod.resize( 256 );
	int			siz	= ezcod_3_10_encode( lat, lon, &cod.front(), cod.size(), 0 );
	if ( siz < 0 ) {
	    std::cout << "encode " << lat << ", " << lon << " failed: " << &cod.front() << std::endl;
	    continue;
	}
	// Reduce the number of symbols to the specified amount.
	int			sizmax	= 0;
	for ( int syms = 0; syms < symbols && sizmax < siz; ++sizmax )
	    if ( ! ::isspace( cod[sizmax] ) && cod[sizmax] != '!' && cod[sizmax] != '.' )
		 ++syms;
	cod.resize( sizmax );

#if defined( DEBUG ) && DEBUG > 0
	std::cout
	    << "encode " << std::setw( 16 ) << lat << ", " << std::setw( 16 ) << lon
	    << " == " << cod
	    << " == " << ezpwd::ezcod<1,9>( ezpwd::ezcod<1,9>( lat, lon ).encode() ) << std::endl;
#endif
	double			lat_o;
	double			lon_o;
	double			acc_o;
	cod += char( 0 );
	cod.resize( 256 );
	int			cnf	= ezcod_3_10_decode( &cod.front(), cod.size(), &lat_o, &lon_o, &acc_o );
	if ( cnf < 0 ) {
	    std::cout << "decode " << lat_o << ", " << lon_o << " failed: " << &cod.front() << std::endl;
	    continue;
	}
	// accuracy, maximum seen.  The maximum linear error seen should always
	// be within the maximum accuracy!
	auto			&lat_acc_lon_i	= lat_acc[lon_i];
	lat_acc_lon_i.second	        = std::max( acc_o, lat_acc_lon_i.second );
	auto			&lon_acc_lat_i	= lon_acc[lat_i];
	lon_acc_lat_i.second	        = std::max( acc_o, lon_acc_lat_i.second );

	double			lat_d	= lat_o - lat;
	double			lat_e	= fabs( lat_d );
	double			lon_d	= lon_o - lon;
	double			lon_e	= fabs( lon_d );

	// compute how many meters the signed and absolute error corresponds to
	double			lon_circ= 1 * M_PI * 6371000;
	double 			lat_dm	= lon_circ * lat_d / 180;
	double 			lat_em	= lon_circ * lat_e / 180;
	double			lat_circ= 2 * M_PI * 6371000 * std::cos( lat * M_PI / 180 );
	double			lon_dm	= lat_circ * lon_d / 360;
	double			lon_em	= lat_circ * lon_e / 360;

	// absolute error.
	auto			&lat_err_lon_i	= lat_err[lon_i];
	lat_err_lon_i.second		       += ( lat_em - lat_err_lon_i.second ) / ++lat_err_lon_i.first;
	auto			&lon_err_lat_i	= lon_err[lat_i];
	lon_err_lat_i.second		       += ( lon_em - lon_err_lat_i.second ) / ++lon_err_lat_i.first;
	//std::cout << "decode " << lat << " --> " << lat_o << " w/ " << lat_em << "m. error ==> " << lat_err_lon_i.second << "m. avg." << std::endl;
	//std::cout << "decode " << lon << " --> " << lon_o << " w/ " << lon_em << "m. error ==> " << lon_err_lat_i.second << "m. avg." << std::endl;

	// signed error.  Should approach 0.
	auto			&lat_dif_lon_i	= lat_dif[lon_i];
	lat_dif_lon_i.second		       += ( lat_dm - lat_dif_lon_i.second ) / ++lat_dif_lon_i.first;
	auto			&lon_dif_lat_i	= lon_dif[lat_i];
	lon_dif_lat_i.second		       += ( lon_dm - lon_dif_lat_i.second ) / ++lon_dif_lat_i.first;

	// linear error.  Should approach 1/2 computed accuracy.
	double			lin_em		= sqrt( lat_dm * lat_dm + lon_dm * lon_dm );
	auto		       &lat_all_lon_i	= lat_all[lon_i];
	lat_all_lon_i.second		       += ( lin_em - lat_all_lon_i.second ) / ++lat_all_lon_i.first;
	auto		       &lon_all_lat_i	= lon_all[lat_i];
	lon_all_lat_i.second		       += ( lin_em - lon_all_lat_i.second ) / ++lon_all_lat_i.first;

	// linear error, maximum seen
	auto		       &lat_max_lon_i	= lat_max[lon_i];
	lat_max_lon_i.second		        = std::max( lin_em, lat_max_lon_i.second );
	auto		       &lon_max_lat_i	= lon_max[lat_i];
	lon_max_lat_i.second		        = std::max( lin_em, lon_max_lat_i.second );
    }
    std::cout << "Longitude avg error, signed difference, total linear, maximum and reported accuracy (at integer Latitudes): " << std::endl;
    for ( int lat_i = 90; lat_i >= -90; --lat_i ) {
	std::cout
	    << std::setw(  5 ) << lat_i << ": "
	    << std::setw( 16 ) << lon_err[lat_i].second << ", " 
	    << std::setw( 16 ) << lon_dif[lat_i].second << ", "
	    << std::setw( 16 ) << lon_all[lat_i].second << " ( "
	    << std::setw( 16 ) << lon_all[lat_i].second * 3.28084 << "ft.), "
	    << std::setw( 16 ) << lon_max[lat_i].second << ", "
	    << std::setw( 16 ) << lon_acc[lat_i].second << " ( "
	    << std::setw( 16 ) << lon_acc[lat_i].second * 3.28084 << "ft.)"
	    << ( abs( lat_i ) == 66
		 ? ( lat_i < 0 ? " polar antarctic" : " polar arctic")
		 : ( abs( lat_i ) == 23
		     ? ( lat_i < 0 ? " tropic of capricorn" : " tropic of cancer" )
		     : ( abs( lat_i ) == 90
			 ? ( lat_i < 0 ? " south pole" : " north pole" )
			 : ( lat_i == 0 
			     ? " equator" 
			     : "" ))))
	    << std::endl;
	if ( assert.ISTRUE( lon_max[lat_i].second <= lon_acc[lat_i].second, "Max linear error by Latitude exceeds computed accuracy" ))
	    std::cout << assert << std::endl;
    }
    std::cout << "Latitude error, signed difference, total linear, maximum and reported accuracy (at integer Longitudes): " << std::endl;
    for ( int lon_i = -180; lon_i <= 180; ++lon_i ) {
	std::cout
	    << std::setw(  5 ) << lon_i << ": "
	    << std::setw( 16 ) << lat_err[lon_i].second << ", " 
	    << std::setw( 16 ) << lat_dif[lon_i].second << ", "
	    << std::setw( 16 ) << lat_all[lon_i].second << " ( "
	    << std::setw( 16 ) << lat_all[lon_i].second * 3.28084 << "ft.), "
	    << std::setw( 16 ) << lat_max[lon_i].second << ", "
	    << std::setw( 16 ) << lat_acc[lon_i].second << " ("
	    << std::setw( 16 ) << lat_acc[lon_i].second * 3.28084 << "ft.)"
	    << std::endl;
	if ( assert.ISTRUE( lat_max[lon_i].second <= lat_acc[lon_i].second, "Max linear error by Longitude exceeds computed accuracy" ))
	    std::cout << assert << std::endl;

    }

    return assert.failures ? 1 : 0;
}
