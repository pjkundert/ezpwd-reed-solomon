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

// 
// Instantiate some of the required vector types used by the en/decode APIs
// 
%include "std_vector.i"
namespace std {
    %template(error_position)	vector<int>;
    %template(uint8_vector)	vector<unsigned char>;
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
    %template(encoded) encoded<std::vector<unsigned char>>;
    %template(decoded) decoded<std::vector<unsigned char>>;
};

// Ship a few 8-bit bch/BCH<...> class template instantiations
// 
// BCH Codecs Available (in bits)
//        size of
//        codeword
//        |  data
//        |  payload
//        |    |    bit error
//        |    |    capacity
//        |    |    |           parity
//        |    |    |           bits  bytes
//        |    |    |            |     |
// BCH( 255, 247,   1 ); ECC =   8 /   1
// BCH( 255, 239,   2 ); ECC =  16 /   2
// BCH( 255, 231,   3 ); ECC =  24 /   3
// BCH( 255, 223,   4 ); ECC =  32 /   4
// BCH( 255, 215,   5 ); ECC =  40 /   5
// BCH( 255, 207,   6 ); ECC =  48 /   6
// BCH( 255, 199,   7 ); ECC =  56 /   7
// BCH( 255, 191,   8 ); ECC =  64 /   8
// 
// (add more here, if desired; see bch_test output for possibilites)
%template(bch_255_1)		ezpwd::bch<255,1>;
%template(bch_255_2)		ezpwd::bch<255,2>;
%template(bch_255_3)		ezpwd::bch<255,3>;
%template(bch_255_4)		ezpwd::bch<255,4>;
%template(bch_255_5)		ezpwd::bch<255,5>;
%template(bch_255_6)		ezpwd::bch<255,6>;
%template(bch_255_7)		ezpwd::bch<255,7>;
%template(bch_255_8)		ezpwd::bch<255,8>;

%template(BCH_255_247_1)	ezpwd::BCH<255,247,1>;
%template(BCH_255_239_2)	ezpwd::BCH<255,239,2>;
%template(BCH_255_231_3)	ezpwd::BCH<255,231,3>;
%template(BCH_255_223_4)	ezpwd::BCH<255,223,4>;
%template(BCH_255_215_5)	ezpwd::BCH<255,215,5>;
%template(BCH_255_207_6)	ezpwd::BCH<255,207,6>;
%template(BCH_255_199_7)	ezpwd::BCH<255,199,7>;
%template(BCH_255_191_8)	ezpwd::BCH<255,191,8>;
