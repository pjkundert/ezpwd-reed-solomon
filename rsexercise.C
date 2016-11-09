
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
    exercise( ezpwd::RS_CCSDS<255,223>(), 100 );
    exercise( ezpwd::RS_CCSDS<255,239>(), 100 );
    exercise( ezpwd::RS_CCSDS_DUAL<255,223>(), 100 );
    exercise( ezpwd::RS_CCSDS_DUAL<255,239>(), 100 );
    exercise( ezpwd::RS<511,511-32>(), 10 );
    exercise( ezpwd::RS<1023,1023-32>(), 10 );
    exercise( ezpwd::RS<65535,65535-32>(), 2 );
    exercise( ezpwd::RS<65535,65535-256>(), 2 );
}
