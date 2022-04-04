/* Exercise an RS codec a specified number of times using random
 * data and error patterns
 *
 * Copyright 2002 Phil Karn, KA9Q
 * May be used under the terms of the GNU General Public License (GPL)
 */
#define FLAG_ERASURE 1 /* Randomly flag 50% of errors as erasures */

#include <iostream>
#include <iomanip>
#include <random>

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include <ezpwd/rs>
#include <ezpwd/timeofday>

/* Exercise the RS codec passed as an argument */
template < typename data_t, unsigned SYM, unsigned NROOTS, unsigned FCS, unsigned PRIM, unsigned POLY, bool DUAL >
int exercise( const ezpwd::reed_solomon<data_t, SYM, NROOTS, FCS, PRIM, ezpwd::gfpoly<SYM,POLY>, DUAL> &rs, int trials=1 )
{
    const unsigned NN = ( 1 << SYM ) - 1;
    data_t block[NN],tblock[NN];
    unsigned i;
    int errors;

    std::array<unsigned,NN> 	errlocs;
    std::array<unsigned,NROOTS>	derrlocs;
    data_t corrvals[NROOTS];
    int derrors;
    unsigned errval,errloc;
    unsigned erasures;
    unsigned tee;
    int decoder_errors = 0;

    struct timeval start;
    struct timeval end;

    //std::default_random_engine	rnd_gen( (unsigned int)time( 0 ));
    std::minstd_rand		rnd_gen( (unsigned int)time( 0 ));
#define random_between( first, last ) std::uniform_int_distribution<unsigned>(( first ), ( last ))
    auto			random_bool	= random_between( 0, 1 );
    auto			random_NROOTS_1	= random_between( 0, NROOTS - 1 );
    auto			random_NN	= random_between( 0, NN - 1 );
    auto			random_25	= random_between( 0, 25 );

    /*
     * Collect stats on 3 types of decodes; no errors, < 1/2 correction capacity, > 1/2 correction
     * capacity.  Also, on 4 percentages of padding symbols vs. capacity <25%, <50%, <75% and <100%.
     * We must perform each type 'kind' of encode/decode in a block, because it is not possible to
     * collect CPU time in resolutions smaller than clock ticks; typically on the order of 1/1000th
     * of a second...
     */
    int decusecs[4][3] = { { 0, 0, 0 },
			   { 0, 0, 0 },
			   { 0, 0, 0 },
			   { 0, 0, 0 } };
    int decblcks[4][3] = { { 0, 0, 0 },
			   { 0, 0, 0 },
			   { 0, 0, 0 },
			   { 0, 0, 0 } };
    double avgerror[4][3]  = { { 0, 0, 0 },
			       { 0, 0, 0 },
			       { 0, 0, 0 },
			       { 0, 0, 0 } };
    double avgerasu[4][3]  = { { 0, 0, 0 },
			       { 0, 0, 0 },
			       { 0, 0, 0 },
			       { 0, 0, 0 } };
    int errknd;
    const char * errstr[3] = {
	"no errors",
	"< 1/2 err",
	">=1/2 err"
    };

    int pad;
    int padpct;
    int padknd;
    const char *padstr[4] = {
	"<25%  pad",
	"<50%  pad",
	"<75%  pad",
	"<100% pad"
    };

    int tri;

    for ( padknd = 0; padknd < 4; ++padknd ) for ( errknd = 0; errknd < 3; ++errknd ) {

	    start		= ezpwd::timeofday();

#if defined( DEBUG ) && DEBUG >= 1
	    trials	= 10;
#endif

	    for ( tri = 0; tri < trials; ++tri ) {

		/* Test up to the error correction capacity of the code */
		switch ( errknd ) {
		case 0: default:	/* no errors */
		    tee		= 0;
		    break;
		case 1:			/* <= 1/2 correction capacity */	
		    tee		= random_NROOTS_1( rnd_gen ) / 2 + 1;
		    break;
		case 2:			/* > 1/2 correction capacity */	
		    tee		= random_NROOTS_1( rnd_gen ) / 2 + NROOTS - ( NROOTS / 2 );
		    break;
		}

		/*
		 * Allow up to the maximum amount of padding.  If NN == 255 and NROOTS == 32, then the
		 * number of non-parity symbols in the codeword is 255 - 32 == 223.  We can allow up to 222
		 * symbols of padding.  padknd specifies the allowable percentage range for this test...
		 */
		padpct	= padknd * 25 + random_25( rnd_gen );/* 0 - 100% */
		pad	= padpct * ( NN - NROOTS - 1) / 100;	/* as little as 1 data symbol */
		auto		random_NN_pad_1	= random_between( 0, NN - pad - 1 );

		/* Load block with random data and encode */
		memset( block, 0, sizeof block );
		for(i=pad;i<NN-NROOTS;i++)
		    block[i] = random_NN( rnd_gen );

		// Encode the block; pad+data+parity
		rs.encode( block+pad, NN-NROOTS-pad, block+NN-NROOTS);

#if defined( DEBUG ) && DEBUG >= 2
		std::cout
		    << "Original: " << std::endl
		    << std::vector<uint8_t>( block+pad, block+NN ) << std::endl;
#endif

		/* Make temp copy, seed with errors */
		memcpy(tblock,block,sizeof(tblock));
		errlocs.fill( 0 );	// memset(errlocs,0,sizeof(errlocs));
		derrlocs.fill( 0 );	// memset(derrlocs,0,sizeof(derrlocs));

		/*
		 * RS codes can correct t errors or 2t erasures.  In other words, "errors * 2 + erasures <
		 * 2t".  So, we loop while we have room for at least 1 more erasure, and still come in at "<
		 * 2t".  Stop adding errors/erasures if we reach the number of non-pad symbols; this will be
		 * automatic, because the capacity will always be less than the number of non-padding
		 * symbols!
		 */
		erasures = 0;
		errors   = 0;
		std::vector<uint8_t>	errident( NN-pad, ' ' );
		for( i = 0; ( 2 * errors + erasures + 1 ) <= tee; i++ ) {
		    do {
			errval = random_NN( rnd_gen );
		    } while(errval == 0); /* Error value must contain at least one non-zero bit */

		    do {
			errloc = random_NN_pad_1( rnd_gen ); /* errloc must be beyond end of pad */
		    } while(errlocs[errloc] != 0); /* Must not choose the same location twice */

		    errlocs[errloc] = 1;

#if FLAG_ERASURE
		    if ( random_bool( rnd_gen ) /* 50-50 chance, or only room for 1 more erasure */
			 || (( 2 * errors + erasures + 1 ) >= tee )) {
			derrlocs[erasures++] = errloc;
			errident[errloc] = '_';
		    } else
#endif
			{
			    ++ errors;
			    errident[errloc] = '?';
			}
		    // If a symbol marked as an erasure is not actually different from the computed
		    // value, it won't be returned in the count (nor included in the
		    // derrlocs/corrvals arrays.  So corrupt erasures to keep derrors consistent.
		    tblock[pad+errloc] ^= errval;	/* erasure or error; corrupt the symbol...*/
		}
#if defined( DEBUG ) && DEBUG >= 2
		std::cout
		    << "Errors:   " << errors << ", erasures: " << erasures << std::endl
		    << errident << std::endl;
#endif

#if defined( DEBUG ) && DEBUG >= 1
		std::cout
		    << rs << " decoding (mode: tee == " << std::setw( 3 ) << tee
		    << ", pad == " << padknd
		    << ", err == " << errknd
		    << ") with " << std::setw( 4 ) << errors 
		    << " errors, " << std::setw( 4 ) << erasures
		    << " erasures; " << std::setw( 4 ) << pad << " pad (" << padpct << "%)" 
		    << std::endl;
#endif

		/* Decode the errored block */
		derrors = rs.decode( tblock+pad, NN-NROOTS-pad, tblock+NN-NROOTS,
				     &derrlocs[0], erasures, corrvals );
#if defined( DEBUG ) && DEBUG >= 2
		for ( int e = 0; e < derrors; ++e )
		    if ( derrlocs[e] >= 0 && derrlocs[e] < NN-pad )
			errident[derrlocs[e]] = corrvals[e];
		std::cout
		    << "Corrects: " << derrors << std::endl
		    << errident << std::endl;
#endif
		
		decblcks[padknd][errknd] ++;
		avgerror[padknd][errknd] += errors;
		avgerasu[padknd][errknd] += erasures;

		if(derrors != int( errors + erasures )){
		    std::cout 
			<< rs << " decoder says " << derrors 
			<< " errors, true number is " << errors + erasures
			<< " (" << errors << " errors, " << erasures << " erasures)"
			<< std::endl;
		    decoder_errors++;
		}
		for(int e=0; e < derrors; e++){
		    if(errlocs[derrlocs[e]] == 0){
			std::cout 
			    << rs << " decoder indicates error in location "
			    << derrlocs[e] << " without error"
			    << std::endl;
			decoder_errors++;
		    }
		}
		if(memcmp(tblock,block,sizeof tblock) != 0){
		    decoder_errors++;
		    std::cout
			<< rs << " uncorrected errors! output ^ input:"
			<< std::setfill( '0' ) << std::hex;
		    for(i=pad;i<NN;i++)
			std::cout
			    << std::setw( 2 ) << (unsigned int)(tblock[i] ^ block[i]);
		    std::cout
			<< std::setfill( ' ' ) << std::dec
			<< std::endl;
		}
	    }

	    end				= ezpwd::timeofday();
	    timeval		duration= end - start;
	    decusecs[padknd][errknd] += duration.tv_sec * 1000000 + duration.tv_usec;
	}

    for ( padknd = 0; padknd < 4; ++padknd ) {
	for ( errknd = 0; errknd < 3; ++errknd ) {
	    if ( decblcks[padknd][errknd] )
		std::cout 
		    << rs
		    << " Enc/Decoding (" << errstr[errknd] << ", " << padstr[padknd]
		    << "). Bytes: " << std::setw( 4 ) << NN
		    << ", Blocks: " << std::setw( 5 ) << decblcks[padknd][errknd]
		    << " (avg. " << std::setprecision( 2 ) << std::setw( 6 )
		    << avgerror[padknd][errknd] / decblcks[padknd][errknd]
		    << " errors, " << std::setprecision( 2 ) << std::setw( 6 )
		    << avgerasu[padknd][errknd] / decblcks[padknd][errknd]
		    << " erasures), Usecs: " << std::setw( 9 ) << decusecs[padknd][errknd]
		    << " == " << std::setprecision( 0 ) << std::setw( 13 )
		    << (int)((( NN * decblcks[padknd][errknd] ) * 1000.0 )
			     / ( decusecs[padknd][errknd] ? decusecs[padknd][errknd] : -1 ))
		    << " kB/s"
		    << std::endl;
	}
    }

    return decoder_errors;
}
