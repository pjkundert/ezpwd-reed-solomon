
#include <array>
#include <vector>
#include <cctype>

#include <ezpwd/rs>
#include <ezpwd/timeofday>

#include <ezpwd/definitions>	// must be included in one C++ compilation unit

// Phil Karn's implementation
extern "C" {
#include <rs.h>
}

// Schifra implementation
#include "schifra/schifra_galois_field.hpp"
#include "schifra/schifra_sequential_root_generator_polynomial_creator.hpp"
#include "schifra/schifra_reed_solomon_encoder.hpp"
#include "schifra/schifra_reed_solomon_decoder.hpp"

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
        << "Phil Karn's Generic     parity: "
	<< std::vector<uint8_t>( gdata.data() + (TOTAL-ROOTS), gdata.data() + TOTAL ) 
        << std::endl;

    std::array<uint8_t,TOTAL>	cdata( orig );
    encode_rs_8( cdata.data(), cdata.data() + (TOTAL-ROOTS), 0 );
    std::cout
        << "Phil Karn's CCSDS       parity: "
	<< std::vector<uint8_t>( cdata.data() + (TOTAL-ROOTS), cdata.data() + TOTAL ) 
        << std::endl;
    
    ezpwd::RS_CCSDS<TOTAL,TOTAL-ROOTS>
				nrs;
    std::array<uint8_t,TOTAL>	ndata( orig );
    nrs.encode( ndata.data(), nrs.LOAD, ndata.data() + nrs.LOAD );
    std::cout
        << "EZPWD   " << nrs
	<< "     parity: " << std::vector<uint8_t>( ndata.data() + (TOTAL-ROOTS), ndata.data() + TOTAL ) 
        << std::endl;
    if ( gdata != ndata )
	throw std::logic_error( "EZPWD and Karn R-S decoders produced different parity" );


    /* Finite Field Parameters */
    const std::size_t field_descriptor                 =   8;
    const std::size_t generator_polynommial_index      = 120;
    const std::size_t generator_polynommial_root_count =  ROOTS;

    /* Reed Solomon Code Parameters */
    const std::size_t code_length = TOTAL;
    const std::size_t fec_length  = ROOTS;
    const std::size_t data_length = code_length - fec_length;

    // 06 ==> 0x187; see schifra_galois_field.hpp
    /* Instantiate Finite Field and Generator Polynomials */
    schifra::galois::field field(field_descriptor,
				 schifra::galois::primitive_polynomial_size06,
				 schifra::galois::primitive_polynomial06);

    schifra::galois::field_polynomial generator_polynomial(field);

    schifra::sequential_root_generator_polynomial_creator(field,
							  generator_polynommial_index,
							  generator_polynommial_root_count,
							  generator_polynomial);

    /* Instantiate Encoder and Decoder (Codec) */
    schifra::reed_solomon::encoder<code_length,fec_length> srs_encoder(field,generator_polynomial);
    schifra::reed_solomon::decoder<code_length,fec_length> srs_decoder(field,generator_polynommial_index);

    /* Instantiate RS Block For Codec */
    schifra::reed_solomon::block<code_length,fec_length> block;

    // Copy the data into sdata, but copy it into/from a std::string of full R-S PAYLOAD length for
    // the Schifra R-S codec.  If the full RS(CAPACITY,PAYLOAD) wasn't used, we would pad w/ 0 on
    // the FRONT, to put the end of the message right at the beginning of the R-S parity.
    std::array<uint8_t,TOTAL>	sdata( orig );

    std::string			sdata_str( data_length, 0 );
    std::copy( sdata.begin(), sdata.end()-ROOTS, sdata_str.begin() );
    srs_encoder.encode( sdata_str, block );
    std::string			spari_str( fec_length, 0 );
    block.fec_to_string( spari_str );
    std::cout
        << "Schifra RS("
	<< schifra::reed_solomon::encoder<code_length,fec_length>::trait::code_length << ","
	<< schifra::reed_solomon::encoder<code_length,fec_length>::trait::data_length << ","
	<< schifra::reed_solomon::encoder<code_length,fec_length>::trait::fec_length  << ")"
	<< "  parity: " << std::vector<uint8_t>( spari_str.begin(), spari_str.end() ) 
        << std::endl;
    // Schifra parity symbols will differ; I can't figure out how to configure non-default
    // polynomial root indices...  Copy the computed data and parity back to sdata.
    for ( size_t i = 0; i < sdata.size(); ++i )
	sdata[i]			= block[i];

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
    if ( gdata != orig )
	throw std::logic_error( "Phil's Generic R-S decoder produced different results" );

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
    if ( cdata != orig )
	throw std::logic_error( "Phil's CCSDS R-S decoder produced different results" );

    // We'll cheat a bit with the Schifra R-S decoder to give it a fighting chance.  Instead of
    // decoding the R-S codeword in-place (in the client's data buffer) like the Phil Karn and EZCOD
    // decoders do, the Schifra decoder wants the data+parity in it's own special
    // schifra::reed_solomon::block.  So, we'll make-believe that the client code will be
    // rearchitected to zero-copy the data; to recieve/transmit it directly from
    // schifra::reed_solomon::block buffers, instead of its own (pre-existing) I/O buffers...
    std::vector<size_t>		seras(nrs.NROOTS);
    int				scorrs	= 0;
    double			stps	= 0;
    {
	timeval			beg	= ezpwd::timeofday();
	timeval			end	= beg;
	end.tv_sec		       += 1;
	int			count	= 0;
	timeval			now;

	// Copy the data into schifra::reed_solomon::block
	for ( size_t i = 0; i < sdata.size(); ++i )
	    block[i]			= sdata[i];
	while (( now = ezpwd::timeofday() ) < end ) {
	    for ( int final = count + 997; count < final; ++count ) {
		uint8_t		err	= (final - count) % 255; // may xor with a zero value
		seras.resize( 1 );
		seras[0]		= count % gdata.size();
		block[seras[0]]	       ^= err;
		if ( ! ( err & 1 ))
		    seras.resize( 0 ); // no erasures; just an error
		if ( ! srs_decoder.decode( block, seras ))
		    throw std::logic_error( "Schifra decoder failed" );
		scorrs			= block.errors_corrected;
		if ( ! scorrs != ! err )
		    throw std::logic_error( "corrections doesn't match error load!" );
	    }
	}
	// Copy the data back out of Schifra's block...
	for ( size_t i = 0; i < sdata.size(); ++i )
	    sdata[i]			= block[i];
	
	double			elapsed	= ezpwd::seconds( now - beg );
	stps				= count / elapsed;
    }
    std::cout 
        << "Schifra    "
	<< "  corrections: "	 	<< scorrs
	<< " at "			<< stps/1000
	<< " kTPS ("			<< abs( stps - gtps ) / gtps * 100
	<< "% "				<< ( stps > gtps ? "faster" : "slower" )
	<< ")"
        << std::endl;
    if ( std::vector<uint8_t>( sdata.begin(), sdata.begin() + TOTAL-ROOTS)
	 != std::vector<uint8_t>( orig.begin(), orig.begin() + TOTAL-ROOTS ))
	throw std::logic_error( "Schifra R-S decoder produced different results" );

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
    if ( ndata != orig )
	throw std::logic_error( "EZPWD R-S decoder produced different results" );

}
