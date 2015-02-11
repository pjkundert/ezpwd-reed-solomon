
#include <array>
#include <vector>
#include <cctype>

#include <ezpwd/rs>
#include <ezpwd/timeofday>

#include <ezpwd/definitions>	// must be included in one C++ compilation unit

extern "C" {
#include <rs.h> // Phil Karn's implementation
}

// Use 32 roots and the CCSDS initialization terms, for compatibility with Phil's fastest
// en/decode_rs_8 fixed-size decoder.
#define TOTAL	255
#define ROOTS	32
#define POLY	0x187
#define FCR	112
#define PRIM	11

int
main()
{
    std::array<uint8_t,TOTAL>	orig;

    std::cout << "RS(" << TOTAL << "," << (TOTAL-ROOTS) << ") data:" << std::endl;
    for ( size_t i = 0; i < (TOTAL-ROOTS); ++i ) {
	orig[i]				= i;
	if ( i and ( i % 32 == 31 or i == (TOTAL-ROOTS)-1 ))
	    std::cout << std::vector<uint8_t>( orig.begin() + i - i % 32, orig.begin() + i + 1 ) << std::endl;
    }

    // ensure that each RS encoder produces the same parity symbols; both Phil Karn's generic codec,
    // his CCSDS-specific implementation, and the new ezpwd/reed_solomon codec.
    void		       *grs	= init_rs_char( 8, POLY, FCR, PRIM, ROOTS, 0 );
    encode_rs_char( grs, orig.data(), orig.data() + (TOTAL-ROOTS) );
    std::array<uint8_t,TOTAL>	gdata( orig );
    std::cout
        << "Phil Karn's  parity: " << std::vector<uint8_t>( gdata.data() + (TOTAL-ROOTS), gdata.data() + TOTAL ) 
        << std::endl;

    std::array<uint8_t,TOTAL>	cdata( orig );
    encode_rs_8( cdata.data(), cdata.data() + (TOTAL-ROOTS), 0 );
    std::cout
        << "Phil's CCSDS parity: " << std::vector<uint8_t>( cdata.data() + (TOTAL-ROOTS), cdata.data() + TOTAL ) 
        << std::endl;
    
    ezpwd::RS_CCSDS<255,TOTAL-ROOTS>	nrs;
    std::array<uint8_t,TOTAL>	ndata( orig );
    nrs.encode( ndata.data(), nrs.LOAD, ndata.data() + nrs.LOAD );
    std::cout
        << nrs << "  parity: " << std::vector<uint8_t>( ndata.data() + (TOTAL-ROOTS), ndata.data() + TOTAL ) 
        << std::endl;
    if ( gdata != ndata )
	throw std::logic_error( "R-S decoders produced different parity" );

    // Get a basic TPS rate for a simple R-S decode with an error, from both encoders.  Specify half
    // of the tests as erasures, half as errors.  Only symbols that actually differ from the
    // computed codeword symbol will be reported as corrected errors/erasures.
    int				geras[ROOTS];
    int				gcorrs	= 0;
    double			gtps	= 0;
    {
	timeval			beg	= ezpwd::timeofday();
	timeval			end	= beg;
	end.tv_sec		       += 1;
	int			count	= 0;
	timeval			now;
	while (( now = ezpwd::timeofday() ) < end ) {
	    for ( int final = count + 997; count < final; ++count ) {
		uint8_t		err	= (final - count ) % 255; // may xor with a zero value
		geras[0]		= count % gdata.size();
		gdata[geras[0]]	       ^= err;
		gcorrs			= decode_rs_char( grs, gdata.data(), geras, err&1 );
		if ( ! gcorrs != ! err )
		    throw std::logic_error( "corrections doesn't match error load!" );
	    }
	}
	double			elapsed	= ezpwd::seconds( now - beg );
	gtps				= count / elapsed;
    }
    std::cout 
	<< "Phil Karn's  corrections: "	<< gcorrs
	<< " at "			<< gtps/1000
	<< " kTPS"
        << std::endl;

    int				ceras[ROOTS];
    int				ccorrs	= 0;
    double			ctps	= 0;
    {
	timeval			beg	= ezpwd::timeofday();
	timeval			end	= beg;
	end.tv_sec		       += 1;
	int			count	= 0;
	timeval			now;
	while (( now = ezpwd::timeofday() ) < end ) {
	    for ( int final = count + 997; count < final; ++count ) {
		uint8_t		err	= (final - count ) % 255; // may xor with a zero value
		ceras[0]		= count % cdata.size();
		cdata[ceras[0]]	       ^= err;
		ccorrs			= decode_rs_8( cdata.data(), ceras, err&1, 0 );
		if ( ! ccorrs != ! err )
		    throw std::logic_error( "corrections doesn't match error load!" );
	    }
	}
	double			elapsed	= ezpwd::seconds( now - beg );
	ctps				= count / elapsed;
    }
    std::cout 
	<< "Phil's CCSDS corrections: "	<< ccorrs
	<< " at "			<< ctps/1000
	<< " kTPS ("			<< abs( ctps - gtps ) / gtps * 100
	<< "% "				<< ( ctps > gtps ? "faster" : "slower" )
	<< ")"
        << std::endl;

    int				neras[nrs.NROOTS];
    int				ncorrs	= 0;
    double			ntps	= 0;
    {
	timeval			beg	= ezpwd::timeofday();
	timeval			end	= beg;
	end.tv_sec		       += 1;
	int			count	= 0;
	timeval			now;
	while (( now = ezpwd::timeofday() ) < end ) {
	    for ( int final = count + 997; count < final; ++count ) {
		uint8_t		err	= (final - count) % 255; // may xor with a zero value
		neras[0]		= count % gdata.size();
		ndata[neras[0]]	       ^= err;
		ncorrs			= nrs.decode( ndata.data(), nrs.LOAD,
						      ndata.data() + nrs.LOAD,
						      neras, err&1 );
		if ( ! ncorrs != ! err )
		    throw std::logic_error( "corrections doesn't match error load!" );
	    }
	}
	double			elapsed	= ezpwd::seconds( now - beg );
	ntps				= count / elapsed;
    }
    std::cout 
        << nrs << "  corrections: " 	<< ncorrs
	<< " at "			<< ntps/1000
	<< " kTPS ("			<< abs( ntps - gtps ) / gtps * 100
	<< "% "				<< ( ntps > gtps ? "faster" : "slower" )
	<< ")"
        << std::endl;
    if ( gdata != ndata or gdata != orig )
	throw std::logic_error( "R-S decoders produced different results" );
}
