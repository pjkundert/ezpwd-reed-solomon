
#include <iostream>
#include <iomanip>
#include <cctype>
#include <cstring>
#include <array>
#include <vector>

#include <rs>
#include <exercise.H>

int main() 
{
    exercise( RS_255( 253 )(), 100 );
    exercise( RS_255_CCSDS( 255-2 )(), 100 );
    exercise( RS_255_CCSDS( 255-4 )(), 100 );
    exercise( RS_255_CCSDS( 255-8 )(), 100 );
    exercise( RS_255_CCSDS( 255-16 )(), 100 );
}
