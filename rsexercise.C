
#include <iostream>
#include <iomanip>
#include <cctype>
#include <cstring>
#include <array>
#include <vector>

#include <ezpwd/rs>
#include "exercise.H"

int main() 
{
    exercise( RS_255( 253 )(), 100 );
    exercise( RS_255_CCSDS( 255-2 )(), 100 );
    exercise( RS_255_CCSDS( 255-4 )(), 100 );
    exercise( RS_255_CCSDS( 255-8 )(), 100 );
    exercise( RS_255_CCSDS( 255-16 )(), 100 );
    exercise( RS_511( 511-32 )(), 10 );
    exercise( RS_1023( 1023-32 )(), 10 );
    exercise( RS_65535( 65535-32 )(), 2 );
    exercise( RS_65535( 65535-256 )(), 2 );
}
