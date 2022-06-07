
// 
// cutone.C		-- CUT unit tests
// 

#include <ezpwd/cut>

#include <math.h>

namespace cut {
    //
    // Create a test suite, which chains itself automatically
    // beneath cut::root.  Don't clutter up your own namespace!
    // 
    test			root( "Simple C++ Unit Test" );

    //
    // Create class derived from cut::test, and defines its run() method. The
    // instance chains itself automatically beneath the given cut::test.
    // 
    CUT( cut::root, SqrtTest, "Simple sqrt() Test" ) {
	assert.out() << "tests sqrt(2) == 1.4 +/- 0.1" << std::endl;
	assert.ISEQUALDELTA( sqrt( 2.0 ), 1.4, 0.1 );
    }
}

int
main(
    int,
    char 	       	      ** )
{
    // 
    // Create cut::runner instances, and run all tests, with HTML output.
    // Return 0 on success, ! 0 on failure. 
    // 
    cut::htmlrunner 		html( std::cout );	// use cut::runner for textual output
    return ! html.run();
}
 
