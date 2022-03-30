/* Exercise an RS codec a specified number of times using random
 * data and error patterns
 *
 * Copyright 2002 Phil Karn, KA9Q
 * May be used under the terms of the GNU General Public License (GPL)
 */
#define FLAG_ERASURE 1 /* Randomly flag 50% of errors as erasures */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#define      CLOCK
#if defined( CLOCK )
#  include <time.h>
#else
#  include <sys/time.h>
#endif

#ifdef FIXED
#include "fixed.h"
#define EXERCISE exercise_8
#define ENCODE_RS encode_rs_8
#define DECODE_RS decode_rs_8
#elif defined(CCSDS)
#include "ccsds.h"
#define EXERCISE exercise_ccsds
#define ENCODE_RS encode_rs_ccsds
#define DECODE_RS decode_rs_ccsds
#elif defined(BIGSYM)
#include "int.h"
#define EXERCISE exercise_int
#define ENCODE_RS encode_rs_int
#define DECODE_RS decode_rs_int
#define PAD_RS pad_rs_int
#else
#include "char.h"
#define EXERCISE exercise_char
#define ENCODE_RS encode_rs_char
#define DECODE_RS decode_rs_char
#define PAD_RS pad_rs_char
#endif

#ifdef FIXED
#define PRINTPARM printf("(255,223):");
#elif defined(CCSDS)
#define PRINTPARM printf("CCSDS (255,223):");
#else
#define PRINTPARM printf("(%d,%d):",rs->nn,rs->nn-rs->nroots);
#endif

#include "rs-common.h"
#include "fec.h"
// Provided by previous version of Phil's Reed-Solomon library.
void *pad_rs_char( void *p, int pad );
void *pad_rs_int( void *p, int pad );

/* Exercise the RS codec passed as an argument */
int EXERCISE(
#if !defined(CCSDS) && !defined(FIXED)
void *p,
#endif
int trials){
#if !defined(CCSDS) && !defined(FIXED)
  struct rs *rs = (struct rs *)p;
#endif
  data_t block[NN],tblock[NN];
  int i;
  int errors;
  int errlocs[NN];
  int derrlocs[NROOTS];
  int derrors;
  int errval,errloc;
  int erasures;
  int tee;
  int decoder_errors = 0;

#if defined( CLOCK )
  clock_t	 start;
  clock_t	 end;
#else
  struct timeval start;
  struct timeval end;
#endif

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

#if defined( CLOCK )
    start	= clock();
#else
    (void)gettimeofday( &start, NULL );
#endif

#if defined( DEBUG ) && DEBUG >= 1
    trials	= 10;
#endif

    for ( tri = 0; tri < trials; ++tri ) {

      /* Test up to the error correction capacity of the code */
      switch ( errknd ) {
      case 0:			/* no errors */
	tee		= 0;
	break;
      case 1:			/* <= 1/2 correction capacity */	
	tee		= ( random() % NROOTS ) / 2 + 1;
	break;
      case 2:			/* > 1/2 correction capacity */	
	tee		= ( random() % NROOTS ) / 2 + NROOTS - ( NROOTS / 2 );
	break;
      }

      /*
       * Allow up to the maximum amount of padding.  If NN == 255 and NROOTS == 32, then the
       * number of non-parity symbols in the codeword is 255 - 32 == 223.  We can allow up to 222
       * symbols of padding.  padknd specifies the allowable percentage range for this test...
       */
      padpct	= padknd * 25 + random() % 26;	/* 0 - 100% */
      pad	= padpct * ( NN - NROOTS - 1) / 100;

      /* Load block with random data and encode */
      memset( block, 0, sizeof block );
      for(i=0;i<NN-NROOTS-pad;i++)
	block[i] = random() & NN;

#if defined(CCSDS) || defined(FIXED)
      ENCODE_RS(&block[0],&block[NN-NROOTS-pad], pad);
#else
      PAD_RS( rs, pad );
      ENCODE_RS(rs,&block[0],&block[NN-NROOTS-pad]);
#endif

      /* Make temp copy, seed with errors */
      memcpy(tblock,block,sizeof(tblock));
      memset(errlocs,0,sizeof(errlocs));
      memset(derrlocs,0,sizeof(derrlocs));

      /*
       * RS codes can correct t errors or 2t erasures.  In other words, "errors * 2 + erasures <
       * 2t".  So, we loop while we have room for at least 1 more erasure, and still come in at "<
       * 2t".  Stop adding errors/erasures if we reach the number of non-pad symbols; this will be
       * automatic, because the capacity will always be less than the number of non-padding
       * symbols!
       */
      erasures = 0;
      errors   = 0;
      for( i = 0; ( 2 * errors + erasures + 1 ) <= tee; i++ ) {
	do {
	  errval = random() & NN;
	} while(errval == 0); /* Error value must contain at least one non-zero bit */

	do {
	  errloc = ( random() % ( NN - pad )) + pad; /* errloc must be beyond end of pad */
	} while(errlocs[errloc] != 0); /* Must not choose the same location twice */

	errlocs[errloc] = 1;

#if FLAG_ERASURE
	if ( ( random() & 1)	/* 50-50 chance, or only room for 1 more erasure */
	     || (( 2 * errors + erasures + 1 ) >= tee )) {
	  derrlocs[erasures++] = errloc;
	} else
#endif
	{
	    ++ errors;
	}
	tblock[errloc-pad] ^= errval;	/* erasure or error; corrupt the symbol...*/
      }

#if DEBUG >= 1
      PRINTPARM
      printf(" decoding (mode: tee == %3d, pad == %1d, err == %1d) with %4d errors, %4d erasures; %4d pad (%2d%)\n",
	     tee, padknd, errknd,
	     errors, erasures,
	     pad, padpct );
#endif

      /* Decode the errored block */
#if defined(CCSDS) || defined(FIXED)
      derrors = DECODE_RS(tblock,derrlocs,erasures,pad);
#else
      derrors = DECODE_RS(rs,tblock,derrlocs,erasures);
#endif

      decblcks[padknd][errknd] ++;
      avgerror[padknd][errknd] += errors;
      avgerasu[padknd][errknd] += erasures;

      if(derrors != errors + erasures){
	PRINTPARM
	printf(" decoder says %d errors, true number is %d (%d errors, %d erasures)\n",
	       derrors, errors + erasures, errors, erasures );
	decoder_errors++;
      }
      for(i=0;i<derrors;i++){
	if(errlocs[derrlocs[i]] == 0){
	  PRINTPARM
	  printf(" decoder indicates error in location %d without error\n",derrlocs[i]);
	  decoder_errors++;
	}
      }
      if(memcmp(tblock,block,sizeof(tblock)) != 0){
	PRINTPARM
	printf(" uncorrected errors! output ^ input:");
	decoder_errors++;
	for(i=0;i<NN;i++)
	  printf(" %02x",tblock[i] ^ block[i]);
	printf("\n");
      }
    }

#if defined( CLOCK )
    end		= clock();
    decusecs[padknd][errknd] += end - start;
#else
    (void)gettimeofday( &end, NULL );
    decusecs[padknd][errknd] += ( end.tv_sec - start.tv_sec ) * 1000000;
    decusecs[padknd][errknd] += ( end.tv_usec - start.tv_usec );
#endif

  }

  for ( padknd = 0; padknd < 4; ++padknd ) {
    for ( errknd = 0; errknd < 3; ++errknd ) {
      if ( decblcks[padknd][errknd] )
	  printf( "Enc/Decoding (%s, %s). Bytes: %4d, Blocks: %5d (avg. %4.2g errors, %4.2g erasures), Usecs: %9d == %13.0f B/s\n",
		  errstr[errknd],
		  padstr[padknd],
		  NN, decblcks[padknd][errknd],
		  avgerror[padknd][errknd] / decblcks[padknd][errknd],
		  avgerasu[padknd][errknd] / decblcks[padknd][errknd],
		  decusecs[padknd][errknd],
		  ((( NN * decblcks[padknd][errknd] ) * 1000000.0 )
		   / ( decusecs[padknd][errknd] ? decusecs[padknd][errknd] : -1 )));

    }
  }

  return decoder_errors;
}
