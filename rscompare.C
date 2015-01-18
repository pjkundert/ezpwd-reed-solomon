
#include <array>
#include <vector>
#include <cctype>

#include <ezpwd/rs>
#include <ezpwd/timeofday>
extern "C" {
#include <rs.h> // Phil Karn's implementation
}

int
main()
{
    std::array<uint8_t,255>	orig;

    std::cout << "RS(255,253) data:" << std::endl;
    for ( size_t i = 0; i < orig.size() - 2; ++i ) {
	orig[i]				= i;
	if ( i and ( i % 32 == 31 or i == 252 ))
	    std::cout << std::vector<uint8_t>( orig.begin() + i - i%32, orig.begin() + i + 1 ) << std::endl;
    }

    // ensure that each RS encoder produces the same parity symbols
    void		       *ors	= init_rs_char( 8, 0x11d, 1, 1, 2, 0 );
    encode_rs_char( ors, orig.data(), orig.data() + 253 );
    std::array<uint8_t,255>	odata( orig );
    
    RS_255( 253 )		nrs;
    std::array<uint8_t,255>	ndata( orig );
    nrs.encode( ndata.data(), nrs.LOAD, ndata.data() + nrs.LOAD );
    std::cout
        << "Phil Karn's parity: " << std::vector<uint8_t>( odata.data() + 253, odata.data() + 255 ) 
        << std::endl;
    std::cout
        << nrs << " parity: " << std::vector<uint8_t>( ndata.data() + 253, ndata.data() + 255 ) 
        << std::endl;
    if ( odata != ndata )
	throw std::logic_error( "R-S decoders produced different parity" );

    // Get a basic TPS rate for a simple R-S decode with an error, from both encoders.  Specify half
    // of the tests as erasures, half as errors.  Only symbols that actually differ from the
    // computed codeword symbol will be reported as corrected errors/erasures.
    int				oeras[2];
    int				ocorrs	= 0;
    double			otps	= 0;
    {
	timeval			beg	= ezpwd::timeofday();
	timeval			end	= beg;
	end.tv_sec		       += 1;
	int			count	= 0;
	timeval			now;
	while (( now = ezpwd::timeofday() ) < end ) {
	    for ( int final = count + 997; count < final; ++count ) {
		uint8_t		err	= (final - count ) % 255; // may xor with a zero value
		oeras[0]		= count % odata.size();
		odata[oeras[0]]	       ^= err;
		ocorrs			= decode_rs_char( ors, odata.data(), oeras, err&1 );
		if ( ! ocorrs != ! err )
		    throw std::logic_error( "corrections doesn't match error load!" );
	    }
	}
	double			elapsed	= ezpwd::seconds( now - beg );
	otps				= count / elapsed;
    }
    std::cout 
	<< "Phil Karn's corrections: "	<< ocorrs
	<< " at "			<< int(otps)/1000
	<< " kTPS"
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
		neras[0]		= count % odata.size();
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
        << nrs << " corrections: " 	<< ncorrs
	<< " at "			<< int(ntps)/1000
	<< " kTPS ("			<< int( abs( ntps - otps ) * 100 / otps )
	<< "% "				<< ( ntps > otps ? "faster" : "slower" )
	<< ")"
        << std::endl;
    if ( odata != ndata or odata != orig )
	throw std::logic_error( "R-S decoders produced different results" );
}
