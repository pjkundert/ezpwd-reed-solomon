%module ezcod

%{
#include "ezpwd/ezcod"
#include "ezpwd/serialize_definitions"
%}

%include std_string.i
%include std_pair.i

%ignore ::operator<<;
%ignore *::output;
%ignore *::symbols;

%include "exception.i"

%exception {
    try {
        $action
    } catch ( const std::exception &e ) {
        SWIG_exception( SWIG_RuntimeError, e.what() );
    }
}

%include ezpwd/ezcod

%extend ezpwd::ezcod {
    std::string		        __str__()
    {
	std::ostringstream	oss;
	$self->output( oss );
	return oss.str();
    }
};

%template(ezcod_3_10) ezpwd::ezcod<1,9>;
%template(ezcod_3_11) ezpwd::ezcod<2,9>;
%template(ezcod_3_12) ezpwd::ezcod<3,9>;
