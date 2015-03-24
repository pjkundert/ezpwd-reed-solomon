#include "ezcod_python.h"

#include <iostream>

template < size_t P=1, size_t L=9 >
inline std::ostream	       &operator<<(
				    std::ostream	&lhs,
				    const EZCOD<P,L>	&rhs )
{
    return rhs.output( lhs );
}

int main( int, const char ** )
{
    std::cout << EZCOD<1,9>("R3U 08M PVT.D") << std::endl;
}
