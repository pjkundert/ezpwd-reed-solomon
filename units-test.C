
// 
// units-test.C		-- UNITS unit tests
// 

/*
 * Ezpwd Reed-Solomon -- Reed-Solomon encoder / decoder library
 * 
 * Copyright (c) 2022, Dominion Research & Development Corp.
 *
 * Ezpwd Reed-Solomon is free software: you can redistribute it and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.  See the LICENSE file at the top of the
 * source tree.  Ezpwd Reed-Solomon is also available under Commercial license.  The Djelic BCH code
 * under djelic/ and the c++/ezpwd/bch_base wrapper is redistributed under the terms of the GPLv2+,
 * regardless of the overall licensing terms.
 * 
 * Ezpwd Reed-Solomon is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 */

#include <ezpwd/units>
#include <ezpwd/cut>

#include <iostream>
#include <algorithm>
#include <string>
#include <fstream>
#include <sstream>
#include <exception>
#include <set>
#include <deque>
#include <map>

#if defined( __GNUC__ )
#  define UNUSED		__attribute__(( unused ))
#else
#  define UNUSED
#endif


#if defined( TESTSTANDALONE )

// Standalone test suite, if TESTSTANDALONE is defined.  
namespace cut {
    test			root( "Root UNITS unit test suite" );
};

int				main( 
				    int			argc,
				    char const 	      **argv )
{
    // if REQUEST_METHOD environment variable set, assume running as CGI
    bool			cgi	= getenv( "REQUEST_METHOD" );
    cut::runner		       *tests;
    if ( cgi )
	tests				= new cut::htmlrunner( std::cout );
    else
	tests				= new cut::runner( std::cout );

    // run returns true iff all test(s) successful
    if ( argc > 1 ) { 				// Named tests specified?
	for ( int i = 1; i < argc; ++i ) {	// Run each one
	    std::cout << "running test: " << argv[i] << std::endl;
	    if ( ! tests->run( argv[i] )) {
		return 1;
	    }
	}
    } else
	return ! tests->run();			// Run all tests
}

#endif // TESTSTANDALONE


#if defined( TEST )

//
// Unit Tests for Units classes
// 
namespace cut {
    typedef units::type<double>	ud_t;
    ud_t			ud;
    ud_t::imperial		impd( ud );
    ud_t::us			usad( ud );
    ud_t::binary		bind( ud );

    typedef units::type<int> ui_t;
//  ui_t			ui( 1 );		// kg, m, s, etc.
    ui_t			ui( 100, 1000, 10 );	// decagrams, mm, s, (remaining dimensions are 1 per unit) etc.
    ui_t::imperial		impi( ui );
    ui_t::us			usai( ui );

    CUT( root,		Units_tests, 		"Basic Units tests" ) {
	ud_t::Time		t	= ud.Second;
	t 			       *= 10;				// mul/div by scalar sensible
#if ! defined( UNITS_DISABLED )	
	assert.ISEQUALDELTA( t.scalar(), 10.0, 0.001 );
#endif
	t 			       /= 1;
#if ! defined( UNITS_DISABLED )	
	assert.ISEQUALDELTA( t.scalar(), 10.0, 0.001 );
#endif
	t 			       += t;				// add/sub in same dimension sensible
#if ! defined( UNITS_DISABLED )	
	assert.ISEQUALDELTA( t.scalar(), 20.0, 0.001 );
#endif
	t 			       -= t / 2;
#if ! defined( UNITS_DISABLED )	
	assert.ISEQUALDELTA( t.scalar(), 10.0, 0.001 );
#endif
	t			        = - -t;
#if ! defined( UNITS_DISABLED )	
	assert.ISEQUALDELTA( t.scalar(), 10.0, 0.001 );
#endif

	ud_t::Acceleration	a	= ud.Meter * 10 / ( ud.Second * ud.Second );
	ud_t::Velocity		v	= a * t;
	double			kph	= v / ( ud.Meter * 1000 / ud.Hour );
	assert.ISEQUALDELTA( double( kph ), 360.0, 0.001 );

	ud_t::Mass		m	= ud.Kilogram;
	ud_t::Force		f	= m * ud.Gravity;		// 1kg at G == approx. 2.2 Pounds
	assert.ISEQUALDELTA( double( f / impd.Pound ), double( 2.20462 ), 0.0001 );

	ui_t::Mass		mi	= ui.Kilogram;
	ui_t::Force		fi	= mi * ui.Gravity;

#if ! defined( UNITS_DISABLED )	
	assert.ISEQUAL( int( mi.scalar() ), 	      100 );
	assert.ISEQUAL( int( fi.scalar() ),    	     9800 );
	assert.ISEQUAL( ui.Gravity.scalar(), 	       98 );
	assert.ISEQUAL( impi.Mile.scalar(), 	  1609343 );
	assert.ISEQUAL( impi.Pound.scalar(),	     4448 );
#endif

	assert.ISEQUAL( int( fi / impi.Pound ), 	2 );

	// Test the copy/assign operators from same dimension, different types.
	mi				= m;
	ui_t::Mass		mi2( m );
	(void)mi2;
    }

    CUT( Units_tests,	Units_Bits,		"Data Units tests" ) {
	ud_t::Unitless		d	= bind.Byte * 388;
	ud_t::Frequency		r	= d / ( ud.Second * 10 );	// 388 bytes over 10 seconds
	double			bps	= r / bind.BPS;
	assert.ISEQUALDELTA( bps, 310.4, 0.001 );
	double			mbps	= r / bind.MBPS;
	assert.ISEQUALPERCENT( mbps, 310.4/1024/1024, 0.001 );
	assert.ISEQUALPERCENT( double( r / ( ud.Mega * bind.BPS )), 310.4/1000/1000, 0.001 );

	ud_t::Unitless		nibble	= ud.Count * 4;
	assert.ISEQUAL( double( nibble ), 4.0 );

    }

    CUT( Units_tests,	Units_Speed,		"Units Speed Test" ) {

	// Perform 1M conversions from Acre-Feet / Miles to Miles/Gallon (US and Imperial), and Liters/100KM.
	for ( int i = 0; i < 1000000; ++i ) {
	    const ud_t::Length	rod	= impd.Mile / 640;
	    const ud_t::Area	acre	= rod * impd.Mile;

	    ud_t::Efficiency	afpm	= acre * impd.Feet * i / impd.Mile;
	    double		lp100km	= afpm / ( ud.Liter / ( ud.Kilo * ud.Meter * 100 ));
	    assert.ISEQUALPERCENT( lp100km, i * 7.6645e+07, 0.01 );
	    if ( i ) {							// Undefined for denominator 0..
		ud_t::Mileage	mpaf	= ud_t::Unitless( 1 ) / afpm;
		double		mpg	= mpaf / ( impd.Mile / impd.Gallon );
		assert.ISEQUALPERCENT( mpg, 3.68558e-06 / i, 0.01 );

		mpg			= mpaf / ( impd.Mile / usad.Gallon );
		assert.ISEQUALPERCENT( mpg, 3.06888e-06 / i, 0.01 );
	    }
	}
    }

#if ! defined( UNITS_DISABLED )


    CUT( Units_tests,	Units_Conversion,	"Units conversion" ) {
#if defined( DEBUG )
	ui_t::Mass		a( ui.Kilogram ); // 1000 g (see above)
	assert.ISEQUALPERCENT( double(   a          / ui.Kilogram ), double( 1.0  ), 0.001 );
	assert.ISEQUALPERCENT( double( ( a * 1.49 ) / ui.Kilogram ), double( 1.0  ), 0.001 );	// compiler issues int-double conversion warning (expected)
#endif
	ud_t::Mass		b( ud.Kilogram ); // 1.0 kg
	assert.ISEQUALPERCENT( double( ( b * 1.49 ) / ud.Kilogram ), double( 1.49 ), 0.001 );

#if 0
	ud_t::Celsius		c( 20 );
	assert.ISEQUALDELTA( c.scalar(), 20.0, 0.001 );
	ud_t::Temperature	k( c );
	assert.ISEQUALDELTA( k.scalar(), 20.0, 0.001 );
#endif

	// Convert mileage (US Gallons), to MPG (both US and Imperial), and L/100KM.
        typedef units::type<float>
                                sif_t;                                  // SI Units Types, over float
        sif_t	                sif;                                    // SI Constants
        sif_t::us               usf( sif );                             // US/Imperial Constants
        
        sif_t::Mileage          mileage = usf.Mile * 10 / usf.Gallon;
        float                   mpg     = mileage / usad.MPG;

        float                   eff     = sif_t::Unitless( 1 ) / mileage / sif.L_100KM;

        assert.ISEQUALDELTA( double( mpg ),     10.0000, 0.001 );
        assert.ISEQUALDELTA( double( eff ),     23.5215, 0.001 );

        sif_t::imperial         imf( sif );                             // Imperial Constants
        mpg                                     = mileage / imf.MPG;
        assert.ISEQUALDELTA( double( mpg ),     12.0095, 0.001 );
    }
#endif
}

#endif // TEST
