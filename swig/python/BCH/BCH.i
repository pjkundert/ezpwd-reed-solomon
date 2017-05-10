%module BCH

%{
#include "ezpwd/bch"
#include "ezpwd/serialize_definitions"
%}

%include std_string.i
%include std_pair.i

// 
// The <</output operations don't make sense for Python.  Supply repr/str instead.
// 
%ignore ::operator<<;
%ignore *::output;

// 
// The in-place {en,de}code methods don't make sense for Python; Use the pass-thru {en,de}coded
// methods.  These encode and return a copy (w/ parity added and/or placed), or decode and return a
// valid or corrected copy.  If not able to {en,de}code successfully, they raise an exception.
// 
%ignore *::encode;
%ignore *::decode;

%include "exception.i"

%exception {
    try {
        $action
    } catch ( const std::exception &e ) {
        SWIG_exception( SWIG_RuntimeError, e.what() );
    }
}

%include ezpwd/bch

%extend ezpwd::bch_base {
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
	std::ostringstream	oss;
	$self->output( oss );
	return oss.str();
    }

    // 
    // Support some known container types for {en,de}coded template
    //
    %template(encoded) encoded<std::string>;
    %template(decoded) decoded<std::string>;

};
