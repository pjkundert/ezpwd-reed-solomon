
#include <math.h> // M_PI
#include <cmath>
#include <random>
#include <iostream>
#include <sstream>

#include <cstdint>
#include <map>

#include <ezpwd/rs>
#include <ezpwd/output>
#include <ezpwd/asserter>
#include <ezpwd/ezcod>
#include <ezpwd/serialize>

#include "ezcod.h"		// C API declarations

#if defined( DEBUG )
extern "C" {
#include <rs.h> // Phil Karn's implementation
}
#endif // DEBUG


template < size_t P, size_t L > void
ezcod_exercise( const ezpwd::ezcod<P,L> &ezc )
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

    double			lat	=   53.555556;
    double			lon	= -113.873889;
    std::map<std::pair<size_t,size_t>,std::string>
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
    std::map<std::pair<size_t,size_t>,double>
   				acc	= {{{1, 3}, 2.600 },
					   {{1, 4}, 2.600 },
					   {{1, 5}, 2.600 },
					   {{1, 6}, 2.600 },
					   {{1, 7}, 2.600 },
					   {{1, 8}, 2.600 },
					   {{1, 9}, 2.600 },
					   {{1,10}, 2.600 },
					   {{1,11}, 2.600 },
					   {{1,12}, 2.600 }};

    ezpwd::ezcod_base	      *ezc[] = {
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
    for ( auto &e : ezc ) {
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

    // Excercise the R-S codecs beyond their correction capability.  This test used to report -'ve
    // error correction positions.  Now, computing -'ve correctly fails the R-S decode, as it
    // indicates that the supplied data's R-S Galois field polynomial solution inferred errors in
    // data we *know* is correct -- the effective block of zero data in the pad (unused) area of the
    // R-S codeword's capacity!

    // Correct encoding w/2 parity:        R3U 08M PVT GY
    //                              errors: v      v
    std::string			err2	= "R0U 08M 0VT GY";
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

#if defined ( DEBUG )
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
	int			siz	= ezcod_3_10_encode( lat, lon, &cod.front(), cod.size() );
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
