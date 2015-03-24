%module ezcod

%{
#include "ezcod_python.h"
%}

%include "std_string.i"
%include "ezcod_python.h"

%extend EZCOD {
    std::string		        __str__()
    {
	std::ostringstream	oss;
	$self->output( oss );
	return oss.str();
    }
};

%template(EZCOD_3_10) EZCOD<1,9>;
%template(EZCOD_3_11) EZCOD<2,9>;
%template(EZCOD_3_12) EZCOD<3,9>;

%include "ezpwd/ezcod"

%extend ezpwd::ezcod {
    std::string		        __str__()
    {
	std::ostringstream	oss;
	$self->output( oss );
	return oss.str();
    }
};

%template(ezcod_3_10) ezpwd::ezcod<1,9>;
