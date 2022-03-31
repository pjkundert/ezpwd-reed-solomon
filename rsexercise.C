
#include <iostream>
#include <iomanip>
#include <cctype>
#include <cstring>
#include <array>
#include <vector>

#include <ezpwd/rs>

#include <ezpwd/definitions>	// must be included in one C++ compilation unit

#include "exercise.H"

int main() 
{
    exercise( ezpwd::RS<255,255-2>(), 100 );
    exercise( ezpwd::RS<255,255-4>(), 100 );
    exercise( ezpwd::RS<255,255-5>(), 100 );
    exercise( ezpwd::RS<255,255-8>(), 100 );
    exercise( ezpwd::RS<255,255-16>(), 100 );
    exercise( ezpwd::RS<255,255-32>(), 100 );
    exercise( ezpwd::RS_CCSDS<255,255-32>(), 100 );
    exercise( ezpwd::RS_CCSDS_CONV<255,255-32>(), 100 );
    exercise( ezpwd::RS<511,511-32>(), 10 );
    exercise( ezpwd::RS<1023,1023-32>(), 10 );
    exercise( ezpwd::RS<65535,65535-32>(), 2 );
    exercise( ezpwd::RS<65535,65535-256>(), 2 );
}
