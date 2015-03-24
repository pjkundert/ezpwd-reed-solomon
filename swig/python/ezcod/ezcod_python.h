#include <ezpwd/ezcod>
#include <ezpwd/serialize_definitions> // only included once per executable

// 
// Provide a simplified public EZCOD interface that hides all internal moving parts
// 

template < size_t P=1, size_t L=9 >
class EZCOD
    : protected ezpwd::ezcod<P,L>
{
public:
    using ezpwd::ezcod<P,L>::latitude;
    using ezpwd::ezcod<P,L>::latitude_error;
    using ezpwd::ezcod<P,L>::longitude;
    using ezpwd::ezcod<P,L>::longitude_error;
    using ezpwd::ezcod<P,L>::accuracy;
    using ezpwd::ezcod<P,L>::confidence;
    using ezpwd::ezcod<P,L>::encode;
    using ezpwd::ezcod<P,L>::decode;
    using ezpwd::ezcod<P,L>::output;
    explicit			EZCOD(
				    double	_lat	= 0,
				    double	_lon	= 0 )
				: ezpwd::ezcod<P,L>( _lat, _lon )
    {
	;
    }
    explicit			EZCOD(
				    const std::string  &str )
				: ezpwd::ezcod<P,L>( str )
    {
	;
    }
    virtual		       ~EZCOD()
    {
	;
    }
};
