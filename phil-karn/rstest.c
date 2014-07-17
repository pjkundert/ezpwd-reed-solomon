/* Test the Reed-Solomon codecs
 * for various block sizes and with random data and random error patterns
 *
 * Copyright 2002 Phil Karn, KA9Q
 * May be used under the terms of the GNU General Public License (GPL)
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "rs.h"

int exercise_char(void *,int);
int exercise_int(void *,int);
int exercise_8(int);
int exercise_ccsds(int);

struct {
  int symsize;
  int genpoly;
  int fcs;
  int prim;
  int nroots;
  int ntrials;
} Tab[] = {
  {2, 0x7,     1,   1,  1,  100000 },
  {3, 0xb,     1,   1,  2,  100000 },
  {4, 0x13,    1,   1,  4,  100000 },
  {5, 0x25,    1,   1,  6,  100000 },
  {6, 0x43,    1,   1,  8,  100000 },
  {7, 0x89,    1,   1, 10,  100000 },
  {8, 0x11d,   1,   1,  4,   50000 },
  {8, 0x11d,   1,   1,  8,   50000 },
  {8, 0x11d,   1,   1, 16,   50000 },
  {8, 0x11d,   1,   1, 32,   50000 },
  {8, 0x187,   112,11,  2,   50000 }, /* Duplicates CCSDS codec */
  {8, 0x187,   112,11,  4,   50000 }, /* Duplicates CCSDS codec */
  {8, 0x187,   112,11,  8,   25000 }, /* Duplicates CCSDS codec */
  {8, 0x187,   112,11, 16,   12000 }, /* Duplicates CCSDS codec */
  {8, 0x187,   112,11, 32,    6000 }, /* Duplicates CCSDS codec */
  {8, 0x187,   112,11, 64,    3000 }, /* Duplicates CCSDS codec */
  {9, 0x211,   1,   1, 32,    2000 },
  {10,0x409,   1,   1, 32,    1500 },
  {11,0x805,   1,   1, 32,    1000 },
  {12,0x1053,  1,   1, 32,     500 },
  {13,0x201b,  1,   1, 32,     250 },
  {14,0x4443,  1,   1, 32,     125 },
  {15,0x8003,  1,   1, 32,      60 },
  {16,0x1100b, 1,   1, 32,      30 },
  {0, 0, 0, 0, 0},
};

inline int millidiff( 
    struct timeval     	       *tvend,
    struct timeval     	       *tvbeg )
{
    return ( (int)( tvend->tv_sec - tvbeg->tv_sec ) * 1000
	     + (int)( tvend->tv_usec )
	     - (int)( tvbeg->tv_usec ));
}

int main(){
  void *handle;
  int errs,terrs;
  int i;

  terrs = 0;
  srandom(time(NULL));



  printf("Testing fixed (255,223) RS codec...");
  fflush(stdout);
  errs = exercise_8(10000);
  terrs += errs;
  if(errs == 0){
    printf("OK\n");
  }
  printf("Testing CCSDS standard (255,223) RS codec...");
  fflush(stdout);
  errs = exercise_ccsds(10000);
  terrs += errs;
  if(errs == 0){
    printf("OK\n");
  }

  for(i=0;Tab[i].symsize != 0;i++){
    int nn,kk;

    nn = (1<<Tab[i].symsize) - 1;
    kk = nn - Tab[i].nroots;
    printf("Testing (%3d,%3d) RS codec, %2d bits/symbol, genpoly: 0x%08x, fcs: %3d, prim: %3d, nroots: %3d...\n",
	   nn,kk,
	   Tab[i].symsize, Tab[i].genpoly, Tab[i].fcs, Tab[i].prim, Tab[i].nroots );
    fflush(stdout);
    if(Tab[i].symsize <= 8){
      if((handle = init_rs_char(Tab[i].symsize,Tab[i].genpoly,Tab[i].fcs,Tab[i].prim,Tab[i].nroots,0)) == NULL){
	printf("init_rs_char failed!\n");
	continue;
      }
      errs = exercise_char(handle,Tab[i].ntrials);
    } else {
      if((handle = init_rs_int(Tab[i].symsize,Tab[i].genpoly,Tab[i].fcs,Tab[i].prim,Tab[i].nroots,0)) == NULL){
	printf("init_rs_int failed!\n");
	continue;
      }
      errs = exercise_int(handle,Tab[i].ntrials);
    }
    terrs += errs;
    if(errs == 0){
      printf("OK\n");
    }
    free_rs_char(handle);
  }
  if(terrs == 0)
    printf("All codec tests passed!\n");

  exit(0);
}
