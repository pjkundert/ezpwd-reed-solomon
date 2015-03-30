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
    std::string		        __repr__()
    {
	std::ostringstream	oss;
	oss << '<';
	$self->output( oss );
	oss << '>';
	return oss.str();
    }
    std::string		        __str__()
    {
	return $self->encode( $self->precision ); // default to same precision as supplied
    }
};

// Define the EZCOD Python API classes available
%template(ezcod_3_10)		ezpwd::ezcod<1,9>;
%template(ezcod_3_11)		ezpwd::ezcod<2,9>;
%template(ezcod_3_12)		ezpwd::ezcod<3,9>;

// Add more ezcod_... variants, here:
// eg. 20mm accuracy and up to 5-nines certainty, in 15 symbols:
//%template(ezcod_20mm_13)	ezpwd::ezcod<1,12>;
//%template(ezcod_20mm_14)	ezpwd::ezcod<2,12>;
//%template(ezcod_20mm_15)	ezpwd::ezcod<3,12>;
