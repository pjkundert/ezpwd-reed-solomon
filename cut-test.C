
// 
// cut-test.C		-- CUT unit tests
// 

#include <ezpwd/cut>

#if defined( __GNUC__ )
#  define UNUSED		__attribute__((unused))
#else
#  define UNUSED
#endif


#if defined( TEST )

namespace cut {

#if defined( TESTSTANDALONE )
    test			root( "C++ Unit Test Example" );
#endif

    CUT( root, cut_tests, "CUT" ) {
	assert.out() << name() << std::endl;
    }

    CUT( cut_tests, FirstSuite, "The First Suite" ) {
	assert.out() << "This is the first suite" << std::endl;
	assert.ISTRUE( 1 );
    }

    CUT( FirstSuite, FirstSuiteInternalOne, "The First Suite's Internal Test One" ) {
	assert.out() << "This is the first suite's internal test #1" << std::endl;
	assert.ISEQUAL( 1 + 1, 2 );
	assert.ISNOTEQUAL( 1 + 1, 3 );
    }

    CUT( cut_tests, animaltests, "Animal Tests" ) {
	assert.out() << "Animal tests" << std::endl;
    }

    CUT( animaltests, ducktest, "Duck Test" ) {
	assert.out() << "Monty Python's excellent witch testing protocol..." << std::endl;
	assert.ISEQUAL( 1 - 1 + 1, 1 );
	// assert.ISUNKNOWN( 1 );		// Unknown (must weigh same as wood)
    }

    CUT( animaltests, cowtest, "Cow Test" ) {
	assert.out() << "Cow test" << std::endl;
	assert.ISTRUE( true );			// Succeeds
    }

    CUT( cowtest, dogtest, "Dog Test" ) {
	assert.out() << "Dog test" << std::endl;
	assert.ISEQUAL( 1 + 1, 2 );
	assert.ISFALSE( 1 + 1 == 4 );
    }

    CUT( cut_tests, Cstringtest, "C String Tests" ) {
	assert.out() << "Now testing C string matching..." << std::endl;
	assert.ISEQUAL( "Hello, World!", "Hello, World!" );
	assert.ISFALSE( 0 == strcmp( "Hello, World!", "Hello,  World!" ));
    }

    CUT( cut_tests, stringtest, "String Tests" ) {
	assert.out() << "Now testing std::string matching..." << std::endl;
	assert.ISEQUAL( std::string( "Hello, World!" ), std::string( "Hello, World!" ) );
	std::string	bye( "Goodbye" );
	assert.ISEQUAL( bye, bye );
    }

    // 
    // Example of repeatedly running a chain of test suites.  You can
    // create a new cut::test, and use CUT to define some test(s)
    // chained to it.  Then, you can run the tree of tests repeatedly
    // with different initial conditions (either in a suite with a
    // loop, or just by calling the tree from within different
    // CUT-generated suites, which set up (and tear down) the initial
    // conditions.  Here, we illustrate one (sort of simple and silly)
    // use of this facility.
    // 
    struct mathfuns { 
	const char	       *description;
	double 	      	      (*fun )( double );
	double			arg;
	double			result;
	double			delta;
    };
#   define			MATH( f, a, v ) { # f "( " #a " )", f, a, v, 1e-4 }
    static mathfuns	       *math;
    static mathfuns		mathtests[] = {
	MATH( sqrt,	2.0,	1.4142 ),
	MATH( sqrt,	4.0,	2.0000 ),
	MATH( cos,	1.0,	0.5403 ),
	MATH( sin,	1.0,	0.8414 ),
	MATH( tan,	1.0,	1.5574 ),
	{ 0, 0, 0, 0, 0 }
    };

    // Root of "isolated" tree of tests
    static test			mathsuite( "Repeated Math Test Suite" );

    // Define test(s) rooted in the "isolated" tree
    CUT( mathsuite, mathvalue, "Test Function Value" ) {
	assert.ISEQUALDELTAS( math->fun( math->arg ), math->result, math->delta, math->description );
    }

    // Run the "isolated" tree of tests, with different initial
    // conditions.  This suite is rooted in the "main" cut::root tree
    // of tests, and explicitly runs the "isolated" tree of tests,
    // checking that it succeeds.
    CUT( cut_tests, repeater, "Run All Math Tests" ) {
	for ( math = mathtests; math->description != 0; ++math ) {
	    // Set up your initial conditions here
	    assert.ISTRUE( assert.run( mathsuite ));
	    // Tear down your conditions here
	}
    }

    // Test Exceptions.  Test succeeds if the correct exception is
    // thrown, fails otherwise.
    CUTEXCEPTION( cut_tests, TemplateExceptionTests, const char *, "Template Exception Tests" ) {
	assert.out() << "Throwing \"Hello, World!\"" << std::endl;
	throw "Hello, World!";
    }

    // Test a selection of types.  Templates generate test code in for
    // each type.
    CUT( cut_tests, TypesTest, "Built-in Types Test" ) {
	assert.ISEQUAL(	   true, true );
	assert.ISNOTEQUAL( true, false );
	assert.ISEQUAL( char( 1 ), char( 1 ) );
	assert.ISNOTEQUAL( char( 1 ), char( 2 ) );
	assert.ISEQUALDELTA( char( 1 ), char( 2 ), char( 1 ));
	assert.ISEQUALPERCENT( char( 1 ), char( 2 ), char( 50 ));
	assert.ISEQUAL( (unsigned char)( 1 ), (unsigned char)( 1 ) );
	assert.ISNOTEQUAL( (unsigned char)( 1 ), (unsigned char)( 2 ) );
	assert.ISEQUALDELTA( (unsigned char)( 1 ), (unsigned char)( 2 ), (unsigned char)( 1 ));
	assert.ISNOTEQUALDELTA( (unsigned char)( 4 ), (unsigned char)( 2 ), (unsigned char)( 1 ));
	assert.ISEQUALPERCENT( (unsigned char)( 1 ), (unsigned char)( 2 ), (unsigned char)( 50 ));
	assert.ISEQUALPERCENT( (unsigned char)( 7 ), (unsigned char)( 5 ), (unsigned char)( 50 ));
	assert.ISEQUALPERCENT( (unsigned char)( 250 ), (unsigned char)( 200 ), (unsigned char)( 25 ));
	assert.ISNOTEQUALPERCENT( (unsigned char)( 255 ), (unsigned char)( 200 ), (unsigned char)( 25 ));
	assert.ISEQUAL( short( 1 ), short( 1 ) );
	assert.ISNOTEQUAL( short( 1 ), short( 2 ) );
	assert.ISEQUALDELTA( short( 1 ), short( 2 ), short( 1 ));
	assert.ISEQUALPERCENT( short( 1 ), short( 2 ), short( 50 ));
	assert.ISEQUALDELTA( short( -1 ), short( -2 ), short( 1 ));
	assert.ISEQUALPERCENT( short( -1 ), short( -2 ), short( 50 ));
	assert.ISEQUAL( int( 1 ), int( 1 ) );
	assert.ISNOTEQUAL( int( 1 ), int( 2 ) );
	assert.ISEQUALDELTA( int( 1 ), int( 2 ), 1 );
	assert.ISEQUALPERCENT( int( 1 ), int( 2 ), int( 50 ));
	assert.ISEQUALDELTA( int( -1 ), int( -2 ), 1 );
	assert.ISEQUALPERCENT( int( -1 ), int( -2 ), int( 50 ));
	assert.ISEQUAL( long( 1 ), long( 1 ) );
	assert.ISNOTEQUAL( long( 1 ), long( 2 ) );
	assert.ISEQUALDELTA( long( 1 ), long( 2 ), long( 1 ));
	assert.ISEQUALPERCENT( long( 1 ), long( 2 ), long( 50 ));
	assert.ISEQUALDELTA( long( -1 ), long( -2 ), long( 1 ));
	assert.ISEQUALPERCENT( long( -1 ), long( -2 ), long( 50 ));
	assert.ISEQUAL( float( 1.1 ), float( 1.1 ) );
	assert.ISNOTEQUAL( float( 1.1 ), float( 2.2 ) );
	assert.ISEQUALDELTA( float( 1.1 ), float( 2.2 ), float( 1.11 ));
	assert.ISEQUALPERCENT( float( 1.1 ), float( 2.2 ), float( 51 ));
	assert.ISEQUALDELTA( float( -1.1 ), float( -2.2 ), float( 1.11 ));
	assert.ISEQUALPERCENT( float( -1.1 ), float( -2.2 ), float( 51 ));
	assert.ISEQUAL( double( 1.1 ), double( 1.1 ) );
	assert.ISNOTEQUAL( double( 1.1 ), double( 2.2 ) );
	assert.ISEQUALDELTA( double( 1.1 ), double( 2.2 ), double( 1.11 ));
	assert.ISEQUALPERCENT( double( 1.1 ), double( 2.2 ), double( 51 ));
	assert.ISEQUALDELTA( double( -1.1 ), double( -2.2 ), double( 1.11 ));
	assert.ISEQUALPERCENT( double( -1.1 ), double( -2.2 ), double( 51 ));
	assert.ISEQUALDELTA( (long long)1234567890LL, (long long)1234567891LL, (long long)1LL );
    }
}


#endif // TEST

#if defined( TESTSTANDALONE )

int
main( int, char ** ) {

    // 
    // Run the root test suite, using various runners
    // 
    cut::htmlrunner	html0( std::cout,
			       true,	// sparse
			       true,	// flat
			       true,	// cgi
			       true,   // do not redirect std::cout
			       0 );	// indent
    html0.run();

    std::cout << "<PRE>\nRunning test suite in HTML mode; sparse, flat, redirecting std::cout.\n</PRE>" << std::endl;
    cut::htmlrunner	html1( std::cout,
			       true,	// sparse
			       true,	// flat
			       false,	// not cgi
			       true,    // do redirect std::cout
			       8 );	// indent
    html1.run();

    std::cout << "<PRE>\nRunning test suite in HTML mode; NOT sparse, flat, redirecting std::cout.\n</PRE>" << std::endl;

    cut::htmlrunner	html2( std::cout,
			       false,	// sparse
			       true,	// flat
			       false,	// not cgi
			       true,    // do redirect std::cout
			       0 );	// indent
    html2.run();

    std::cout << "<PRE>\nRunning  ONE suite in HTML mode; NOT sparse, flat, redirecting std::cout.\n</PRE>" << std::endl;
    html2.run( "Cow Test" );

    std::cout << "<PRE>\nRunning   NO suite in HTML mode; NOT sparse, flat, redirecting std::cout.\n</PRE>" << std::endl;
    html2.run( "Invalid Test Suite Name" );

    std::cout << "<PRE>\nRunning test suite in HTML mode; NOT sparse, NOT flat, NOT redirecting std::cout.\n</PRE>" << std::endl;

    cut::htmlrunner	html3( std::cout,
			       false,	// sparse
			       false,	// heirarchical flat
			       false,	// not cgi
			       false,   // do redirect std::cout
			       8 );	// indent
    html3.run();


    std::cout << "<PRE>\nRunning test suite in text mode; sparse\n</PRE>" << std::endl;
    std::cout << "<PRE>" << std::endl;
    cut::runner		text1( std::cout,
			       true ); 	// sparse
    text1.run();
    std::cout << "</PRE>" << std::endl;

    std::cout << "<PRE>\nRunning test suite in text mode; NOT sparse\n</PRE>" << std::endl;
    std::cout << "<PRE>" << std::endl;
    cut::runner		text2( std::cout,
			       false );	// not sparse
    text2.run();
    std::cout << "</PRE>" << std::endl;

    return 0;
}

#endif // TESTSTANDALONE
