
#include <array>
#include <vector>
#include <cctype>
#include <cmath>
#include <functional>

#include <ezpwd/rs>
#include <ezpwd/output>
#include <ezpwd/timeofday>
#include <ezpwd/asserter>

#include <ezpwd/rs_definitions>	// must be included in one C++ compilation unit


// Phil Karn's implementation
extern "C" {
#include <rs.h>
}

template <size_t TOTAL, size_t ROOTS, size_t PAYLOAD=TOTAL-ROOTS>
void				init(
				     std::array<uint8_t,TOTAL> &data,
				     std::function<void (
					 uint8_t	       *, // should be const, but some encoders are badly typed
					 size_t,
					 uint8_t 	       *,
					 size_t
				     )>				encoder
				 )
{
    for ( size_t i = 0; i < PAYLOAD; ++i )
	data[i]				= i;
    for ( size_t i = 0; i < ROOTS; ++i )
	data[PAYLOAD+i]			= 0;
    encoder( data.data(), PAYLOAD, data.data() + PAYLOAD, ROOTS );
}

template <size_t TOTAL, size_t ROOTS, size_t PAYLOAD=TOTAL-ROOTS>
void				dump( std::string what, std::array<uint8_t,TOTAL> &data )
{
    std::cout
	<< std::setw( 24 ) << std::left << what << " RS(" << TOTAL << "," << (TOTAL-ROOTS)
	<< ") w/ " << PAYLOAD << " data, " << ROOTS << " parity:" << std::endl;

    std::cout << "DATA:   " << std::vector<uint8_t>( data.begin(), data.begin() + PAYLOAD ) << std::endl;
    std::cout << "PARITY: " << std::vector<uint8_t>( data.begin() + PAYLOAD, data.begin() + PAYLOAD + ROOTS ) << std::endl;
}


// 
// Test a variety of 8-bit Reed-Solomon codecs.
// - Phil Karn's "FIXED" {en,de}code_rs_8 are specific to the CCSDS polynomial 0x187, w/o dual-basis
// - Phil Karn's "CCSDS" {en,de}code_rs_ccsds only works w/ CCSDS polynomial and RS(255,223), w/ dual-basis encoding
//

//        eg.     255,             2,            253
template <size_t TOTAL, size_t ROOTS, size_t PAYLOAD=TOTAL-ROOTS>
double 				compare(
				    ezpwd::asserter    &assert )
{
    // Get an 'orig' payload.  This data is considered to be in "conventional basis" for normal R-S
    // operations, and is considered to be in "dual basis" for CCSDS operations (ie. it is converted
    // to "conventional basis" by the CCSDS R-S operators de/encoded, and the results restored to
    // "dual basis" on return).
    std::array<uint8_t,TOTAL>	orig;

    // Ensure that each RS encoder produces the same parity symbols; both Phil Karn's generic codec,
    // his CCSDS-specific implementation (for 32 == ROOTS only), and the new ezpwd/reed_solomon
    // codec.  The 'orig' will be the generic reference conventional R-S output; 'corig' will be the
    // reference dual-basis CCSDS R-S output.  The only difference should be in the parity symbol
    // encoding (orig conventional, corig dual basis).
    static const ezpwd::RS<TOTAL,TOTAL-ROOTS>
				nrs;
    void		       *grs	= init_rs_char( 8, nrs.poly(), nrs.fcr(), nrs.prim(), ROOTS, 0 );

    auto karn_encoder		= [grs](
				         uint8_t	       *payload,
					 size_t,
					 uint8_t 	       *parity,
					 size_t
					) -> void
    {
	encode_rs_char( grs, payload, parity );
    };

    init<TOTAL,ROOTS>( orig, karn_encoder );
    //dump<TOTAL,ROOTS>( "Phil Karn's Generic", orig );

    // Get a baseline TPS rate for a simple R-S decode with an error, from Phil's general-purpose
    // decoder.  Specify half of the tests as erasures, half as errors.  Only symbols that actually
    // differ from the computed codeword symbol will be reported as corrected errors/erasures.
    int				gcorrs	= 0;
    double			gtps	= 0;
    {
	int			geras[ROOTS];
	std::array<uint8_t,TOTAL> gdata( orig );
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
		int		numeras	= (ROOTS > 1 ? err&1 : 1 ); // 1 parity? erasure only
		gcorrs			= decode_rs_char( grs, gdata.data(), geras, numeras );
		if ( assert.ISEQUAL( ! gcorrs, ! err ))
		    std::cout
			<< assert << " corrections doesn't match error load!"
			<< std::endl;
	    }
	}
	double			elapsed	= ezpwd::seconds( now - beg );
	gtps				= count / elapsed;

	if ( assert.ISTRUE( gdata == orig ))
	    std::cout
		<< assert << " Phil's Generic R-S decoder produced different results"
		<< std::endl;
    }
    std::cout 
	<< nrs
	<< " (Phil Karn's) corrections: "	<< gcorrs
	<< " at "				<< gtps/1000
	<< " kTPS"
        << std::endl;

    // Finally, test the Ezpwd RS decoder.  Should use the same default polynomial as Phil Karn's
    // generic codec.
    int				ncorrs	= 0;
    double			ntps	= 0;
    {
	std::vector<unsigned>	neras( 1 );
	std::array<uint8_t,TOTAL> ndata( orig );
	timeval			beg	= ezpwd::timeofday();
	timeval			end	= beg;

	end.tv_sec		       += 1;
	int			count	= 0;
	timeval			now;
	while (( now = ezpwd::timeofday() ) < end ) {
	    for ( int final = count + 997; count < final; ++count ) {
		uint8_t		err	= (final - count) % 255; // may xor with a zero value
		neras[0]		= count % ndata.size();
		ndata[neras[0]]	       ^= err;
		unsigned	numeras	= (ROOTS > 1 ? err&1 : 1 ); // 1 parity? erasure only
		ncorrs			= nrs.decode( ndata.data(), nrs.LOAD, ndata.data() + nrs.LOAD, neras.data(), numeras );
		if ( assert.ISEQUAL( ! ncorrs, ! err ))
		    std::cout
			<< assert << " corrections doesn't match error load!"
			<< std::endl;
	    }
	}
	double			elapsed	= ezpwd::seconds( now - beg );
	ntps				= count / elapsed;
	if ( assert.ISTRUE( ndata == orig ))
	    std::cout
		<< assert << " EZPWD R-S decoder produced different results"
		<< std::endl;
    }
    std::cout 
	<< nrs
	<< " (EZPWD's)     corrections: "	<< ncorrs
	<< " at "				<< ntps/1000
	<< " kTPS ("				<< std::abs( ntps - gtps ) / gtps * 100
	<< "% "					<< ( ntps > gtps ? "faster" : "slower" )
	<< ")"
        << std::endl;

    free_rs_char( grs );
    return ( ntps - gtps ) / gtps * 100;
}

int main()
{
    ezpwd::asserter		assert;
    double			avg	= 0;
    int				cnt	= 0;

    avg				       += compare<255,128>( assert );	++cnt;
    avg				       += compare<255, 99>( assert );	++cnt;
    avg				       += compare<255, 64>( assert );	++cnt;
    avg				       += compare<255, 32>( assert );	++cnt;
    avg				       += compare<255, 16>( assert );	++cnt;
    avg				       += compare<255, 13>( assert );	++cnt;
    avg				       += compare<255,  8>( assert );	++cnt;
    avg				       += compare<255,  4>( assert );	++cnt;
    avg				       += compare<255,  3>( assert );	++cnt;
    avg				       += compare<255,  2>( assert );	++cnt;
    avg				       += compare<255,  1>( assert );	++cnt;

    std::cout << std::endl << "RS(255,...) EZPWD vs. Phil Karn's: " << avg/cnt << "% faster (avg.)" << std::endl;

    return assert.failures ? 1 : 0;
}
