
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

// Schifra implementation
#include "schifra/schifra_galois_field.hpp"
#include "schifra/schifra_sequential_root_generator_polynomial_creator.hpp"
#include "schifra/schifra_reed_solomon_encoder.hpp"
#include "schifra/schifra_reed_solomon_decoder.hpp"


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
#if defined( DEBUG )
    dump<TOTAL,ROOTS>( "Phil Karn's Conventional", orig );
#endif
    std::array<uint8_t,TOTAL>	gdata( orig );


    // Now encode w/ EZPWD and Schifra R-S, and ensure we get matching conventional parity symbols.
    std::array<uint8_t,TOTAL>	ndata( orig );
    if ( assert.ISEQUAL( nrs.encode( ndata.data(), PAYLOAD, ndata.data() + PAYLOAD ), int( ROOTS )))
	std::cout
	    << assert << " " << nrs << ".encode didn't return NROOTS"
	    << std::endl;
#if defined( DEBUG )
    dump<TOTAL,ROOTS>( "EZPWD Conventional", orig );
#endif

    if ( assert.ISTRUE( ndata == gdata )) {
	std::cout
	    << assert << "EZPWD and Karn R-S decoders produced different parity"
	    << std::endl;
	dump<TOTAL,ROOTS>( "Phil Karn's", gdata );
	dump<TOTAL,ROOTS>( "EZPWD", ndata );
    }

    // The Schifra R-S codec
    /* Finite Field Parameters */
    const std::size_t field_descriptor                 =   8;
    const std::size_t generator_polynommial_index      = 120;
    const std::size_t generator_polynommial_root_count =  ROOTS;

    /* Reed Solomon Code Parameters */
    const std::size_t code_length = PAYLOAD + ROOTS;
    const std::size_t fec_length  = ROOTS;
    const std::size_t data_length = code_length - fec_length;

    // 06 ==> 0x187; see schifra_galois_field.hpp
    /* Instantiate Finite Field and Generator Polynomials */
    schifra::galois::field field(field_descriptor,
				 schifra::galois::primitive_polynomial_size06,
				 schifra::galois::primitive_polynomial06);

    schifra::galois::field_polynomial generator_polynomial(field);

    bool			sroot = schifra::make_sequential_root_generator_polynomial(
					    field,
					    generator_polynommial_index,
					    generator_polynommial_root_count,
					    generator_polynomial);
    if ( assert.ISTRUE( sroot ))
	std::cout
	    << assert << " Failed to create sequential root generator!"
	    << std::endl;

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
    std::copy( sdata.begin(), sdata.begin() + PAYLOAD, sdata_str.begin() );
    srs_encoder.encode( sdata_str, block );
    std::string			spari_str( fec_length, 0 );
    block.fec_to_string( spari_str );
#if defined( DEBUG )
    std::cout
        << "Schifra RS("
	<< (int)schifra::reed_solomon::encoder<code_length,fec_length>::trait::code_length << ","
	<< (int)schifra::reed_solomon::encoder<code_length,fec_length>::trait::data_length << ","
	<< (int)schifra::reed_solomon::encoder<code_length,fec_length>::trait::fec_length  << ")"
	<< "  parity: " << std::vector<uint8_t>( spari_str.begin(), spari_str.end() ) 
        << std::endl;
#endif
    // Schifra parity symbols will differ; I can't figure out how to configure non-default
    // polynomial root indices...  Copy the computed data and parity back to sdata from block.
    for ( size_t i = 0; i < sdata.size(); ++i )
	sdata[i]			= block[i];
#if defined( DEBUG )
    dump<TOTAL,ROOTS>( "Schifra Conventional", sdata );
#endif
    // Get a baseline TPS rate for a simple R-S decode with an error, from Phil's general-purpose
    // decoder.  Specify half of the tests as erasures, half as errors.  Only symbols that actually
    // differ from the computed codeword symbol will be reported as corrected errors/erasures.
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
    }
    std::cout 
	<< "Phil Karn's  corrections: "	<< gcorrs
	<< " at "			<< gtps/1000
	<< " kTPS"
        << std::endl;
    if ( assert.ISTRUE( gdata == orig )) {
	std::cout
	    << assert << " Phil Karn's Conventional R-S decoder produced different results"
	    << std::endl;
	dump<TOTAL,ROOTS>( "Phil Karn's", gdata );
	dump<TOTAL,ROOTS>( "Original", orig ) ;
    }

    // Test Phil's Fast (8-bit symbol) CCSDS RS(255,223) conventional (not dual-basis) decoder.
    // Remember, it implements the CCSDS polynomial, so produces parity symbols different than the
    // standard RS codec... Phil's fast CCSDS {en,de}code_rs_8 8-bit fixed codec w/ CCSDS 0x187
    // polynomial but w/o dual-basis encoding; will give different parity than the standard
    // RS(255,<PAYLOAD>) encoder w/ 0x11d polynomial.
    if ( ROOTS == 32 ) {
	auto karn_ccsds_conv	= [](
					     uint8_t	       *payload,
					     size_t,
					     uint8_t 	       *parity,
					     size_t
					     ) -> void
	{
	    encode_rs_8( payload, parity, 0 );
	};

	std::array<uint8_t,TOTAL>	fdata;
	init<TOTAL,ROOTS>( fdata, karn_ccsds_conv );
#if defined( DEBUG )
	dump<TOTAL,ROOTS>( "Phil's Conv. CCSDS", fdata );
#endif
	// Ensure EZPWD's RS_CCSDS_CONV conventional encoder produces identical results
	static const ezpwd::RS_CCSDS_CONV<TOTAL,TOTAL-ROOTS>
				ccrs;
	auto ezpwd_ccsds_conv	= [&](
					     uint8_t	       *payload,
					     size_t		paysize,
					     uint8_t 	       *parity,
					     size_t
				) -> void
	{
	    ccrs.encode( payload, paysize, parity );
	};

	std::array<uint8_t,TOTAL> cezpwd;
	init<TOTAL,ROOTS>( cezpwd, ezpwd_ccsds_conv );
#if defined( DEBUG )
	dump<TOTAL,ROOTS>( "EZPWD  Conv. CCSDS", cezpwd );
#endif
	if ( assert.ISTRUE( cezpwd == fdata )) {
	    std::cout
		<< assert << " EZPWD and Karn R-S CCSDS conventional encoders produced different parity"
		<< std::endl;
	    dump<TOTAL,ROOTS>( "Phil's Conv. CCSDS", fdata );
	    dump<TOTAL,ROOTS>( "EZPWD  Conv. CCSDS", cezpwd) ;
	}

	int			feras[ROOTS];
	int			fcorrs	= 0;
	double			ftps	= 0;
	{
	    timeval		beg	= ezpwd::timeofday();
	    timeval		end	= beg;
	    end.tv_sec		       += 1;
	    int			count	= 0;
	    timeval		now;
	    while (( now = ezpwd::timeofday() ) < end ) {
		for ( int final = count + 997; count < final; ++count ) {
		    uint8_t	err	= (final - count ) % 255; // may xor with a zero value
		    feras[0]		= count % fdata.size();
		    fdata[feras[0]]    ^= err;
		    int		numeras	= (ROOTS > 1 ? err&1 : 1 ); // 1 parity? erasure only
		    fcorrs		= decode_rs_8( fdata.data(), feras, numeras, 0 );
		    if ( assert.ISEQUAL( ! fcorrs, ! err ))
			std::cout 
			    << assert <<  " corrections doesn't match error load!"
			    << std::endl;
		}
	    }
	    double		elapsed	= ezpwd::seconds( now - beg );
	    ftps			= count / elapsed;
	}
	std::cout 
	    << "Phil's Fast  corrections: "<< fcorrs
	    << " at "			<< ftps/1000
	    << " kTPS ("		<< std::abs( ftps - gtps ) / gtps * 100
	    << "% "			<< ( ftps > gtps ? "faster" : "slower" )
	    << ")"
	    << std::endl;
    }


    // Phil's CCSDS {en,de}code_rs_ccsds codec uses CCSDS polynomial, and implements dual-basic
    // {en/de}coding, only on RS(255,223), and uses {en,de}code_rs_8 fixed-size codec internally.
    if ( ROOTS == 32 ) {
	// Test CCSDS (255,223) w/ Berleskamp dual-basis encoding.
	auto karn_ccsds		= [](
					     uint8_t	       *payload,
					     size_t,
					     uint8_t 	       *parity,
					     size_t
				) -> void
	{
	    encode_rs_ccsds( payload, parity, 0 );
	};

	std::array<uint8_t,TOTAL>cdata;
	init<TOTAL,ROOTS>( cdata, karn_ccsds );
#if defined( DEBUG )
	dump<TOTAL,ROOTS>( "Phil's CCSDS", cdata );
#endif
	// Ensure EZPWD's RS_CCSDS dual-basis encoder produces identical results
	static const ezpwd::RS_CCSDS<TOTAL,TOTAL-ROOTS>
				crs;
	auto ezpwd_ccsds	= [&](
					     uint8_t	       *payload,
					     size_t		paysize,
					     uint8_t 	       *parity,
					     size_t
				) -> void
	{
	    crs.encode( payload, paysize, parity );
	};

	std::array<uint8_t,TOTAL> cezpwd;
	init<TOTAL,ROOTS>( cezpwd, ezpwd_ccsds );
#if defined( DEBUG )
	dump<TOTAL,ROOTS>( "EZPWD       CCSDS", cezpwd );
#endif
	if ( assert.ISTRUE( cezpwd == cdata )) {
	    std::cout
		<< assert << " EZPWD and Karn R-S CCSDS encoders produced different parity"
		<< std::endl;
	    dump<TOTAL,ROOTS>( "Phil Karn's CCSDS", cdata );
	    dump<TOTAL,ROOTS>( "EZPWD       CCSDS", cezpwd) ;
	}

	// Ensure we always recover the original cdata
	std::array<uint8_t,TOTAL>corig( cdata );

	int			ceras[ROOTS];
	int			ccorrs	= 0;
	double			ctps	= 0;
	{
	    timeval		beg	= ezpwd::timeofday();
	    timeval		end	= beg;
	    end.tv_sec		       += 1;
	    int			count	= 0;
	    timeval		now;
	    while (( now = ezpwd::timeofday() ) < end ) {
		for ( int final = count + 997; count < final; ++count ) {
		    uint8_t	err	= (final - count ) % 255; // may xor with a zero value
		    ceras[0]		= count % (cdata.size()-ROOTS); // Phil's CCSDS decode only fixes payload errors!
		    cdata[ceras[0]]    ^= err;
		    int		numeras	= (ROOTS > 1 ? err&1 : 1 ); // 1 parity? erasure only
		    ccorrs		= decode_rs_ccsds( cdata.data(), ceras, numeras, 0 );
		    if ( assert.ISEQUAL( ! ccorrs, ! err ))
			std::cout 
			    << assert <<  " corrections " << ccorrs << "doesn't match error load " << err << " !"
			    << std::endl;
		}
	    }
	    double		elapsed	= ezpwd::seconds( now - beg );
	    ctps			= count / elapsed;
	}
	std::cout 
	    << "Phil's CCSDS corrections: "<< ccorrs
	    << " at "			<< ctps/1000
	    << " kTPS ("		<< std::abs( ctps - gtps ) / gtps * 100
	    << "% "			<< ( ctps > gtps ? "faster" : "slower" )
	    << ")"
	    << std::endl;
	if ( assert.ISTRUE( cdata == corig )) {
	    std::cout
		<< assert << " Phil's CCSDS R-S decoder produced different results"
		<< std::endl;
	    dump<TOTAL,ROOTS>( "Phil's CCSDS corrected", gdata );
	    dump<TOTAL,ROOTS>( "Phil's CCSDS original", orig ) ;
	}
    }

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
		int		numeras	= (ROOTS > 1 ? err&1 : 1 ); // 1 parity? erasure only
		if ( ! numeras )
		    seras.resize( 0 ); // no erasures; just an error
		if ( assert.ISTRUE( srs_decoder.decode( block, seras )))
		    std::cout
			<< assert << " Schifra decoder failed"
			<< std::endl;
		scorrs			= block.errors_corrected;
		if ( assert.ISEQUAL( ! scorrs, ! err ))
		    std::cout
			<< assert <<  " corrections doesn't match error load!"
			<< std::endl;
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
	<< " kTPS ("			<< std::abs( stps - gtps ) / gtps * 100
	<< "% "				<< ( stps > gtps ? "faster" : "slower" )
	<< ")"
        << std::endl;
    if ( assert.ISTRUE( std::vector<uint8_t>( sdata.begin(), sdata.begin() + TOTAL-ROOTS )
		     == std::vector<uint8_t>(  orig.begin(),  orig.begin() + TOTAL-ROOTS ))) {
	std::cout
	    << assert << " Schifra and Phil-Karn R-S decoder produced different results"
	    << std::endl;
	dump<TOTAL,ROOTS>( "Phil's original   (data only!)", orig );
	dump<TOTAL,ROOTS>( "Schifra corrected (data only!)", sdata );
    }

    unsigned			neras[nrs.NROOTS];
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
		int		numeras	= (ROOTS > 1 ? err&1 : 1 ); // 1 parity? erasure only
		ncorrs			= nrs.decode( ndata.data(), nrs.LOAD, ndata.data() + nrs.LOAD, neras, numeras );
		if ( assert.ISEQUAL( ! ncorrs, ! err ))
		    std::cout
			<< assert << " corrections doesn't match error load!"
			<< std::endl;
	    }
	}
	double			elapsed	= ezpwd::seconds( now - beg );
	ntps				= count / elapsed;
    }
    std::cout 
        << nrs << "  corrections: " 	<< ncorrs
	<< " at "			<< ntps/1000
	<< " kTPS ("			<< std::abs( ntps - gtps ) / gtps * 100
	<< "% "				<< ( ntps > gtps ? "faster" : "slower" )
	<< ")"
        << std::endl;
    if ( assert.ISTRUE( ndata == orig )) {
	std::cout
	    << assert << " EZPWD " << nrs << " and Phil's R-S decoder produced different results"
	    << std::endl;
	dump<TOTAL,ROOTS>( "EZPWD corrected", ndata );
	dump<TOTAL,ROOTS>( "Phil's original", orig ) ;
    }

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
