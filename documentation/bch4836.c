/*
 * File:    bch4836.c 
 * Author:  Robert Morelos-Zaragoza
 *
 * %%%%%%%%%%% Encoder/Decoder for a (48, 36, 5) binary BCH code %%%%%%%%%%%%%
 *
 *	This code is used in control channels for cellular TDMA in the U.S.A.
 *
 *	In this specific case, there is no need to use the Berlekamp-Massey
 *	algorithm, since the error locator polynomial is of at most degree 2.
 *	Instead, we simply solve by hand two simultaneous equations to give
 * 	the coefficients of the error locator polynomial in the case of two 
 *	errors. In the case of one error, the location is given by the first
 *	syndrome.
 *
 *	This program derivates from the original bch2.c, which was written
 *	to simulate the encoding/decoding of primitive binary BCH codes.
 *	Part of this program is adapted from a Reed-Solomon encoder/decoder
 *	program,  'rs.c', to the binary case. 
 *
 *	rs.c by Simon Rockliff, University of Adelaide, 21/9/89 
 *	bch2.c by Robert Morelos-Zaragoza, University of Hawaii, 5/19/92 
 *
 * COPYRIGHT NOTICE: This computer program is free for non-commercial purposes.
 * You may implement this program for any non-commercial application. You may 
 * also implement this program for commercial purposes, provided that you
 * obtain my written permission. Any modification of this program is covered
 * by this copyright.
 *
 * %%%% Copyright 1994 (c) Robert Morelos-Zaragoza. All rights reserved. %%%%%
 *
 * m = order of the field GF(2**6) = 6
 * n = 2**6 - 1 - 15 = 48 = length 
 * t = 2 = error correcting capability 
 * d = 2*t + 1 = 5 = designed minimum distance 
 * k = n - deg(g(x)) = 36 = dimension 
 * p[] = coefficients of primitive polynomial used to generate GF(2**6)
 * g[] = coefficients of generator polynomial, g(x)
 * alpha_to [] = log table of GF(2**6) 
 * index_of[] = antilog table of GF(2**6)
 * data[] = coefficients of data polynomial, i(x)
 * bb[] = coefficients of redundancy polynomial ( x**(12) i(x) ) modulo g(x)
 * numerr = number of errors 
 * errpos[] = error positions 
 * recd[] = coefficients of received polynomial 
 * decerror = number of decoding errors (in MESSAGE positions) 
 *
 */

#include <math.h>
#include <stdio.h>

int             m = 6, n = 63, k = 36, t = 2, d = 5;
int				length = 48;
int             p[7];		/* irreducible polynomial */
int             alpha_to[64], index_of[64], g[13];
int             recd[48], data[36], bb[13];
int             numerr, errpos[64], decerror = 0;
int             seed;


void 
read_p()
/* Primitive polynomial of degree 6 */
{
	register int    i;
    p[0] = p[1] = p[6] = 1; p[2] = p[3] = p[4] = p[5] =0;
}


void 
generate_gf()
/*
 * generate GF(2**m) from the irreducible polynomial p(X) in p[0]..p[m]
 * lookup tables:  index->polynomial form   alpha_to[] contains j=alpha**i;
 * polynomial form -> index form  index_of[j=alpha**i] = i alpha=2 is the
 * primitive element of GF(2**m) 
 */
{
	register int    i, mask;
	mask = 1;
	alpha_to[m] = 0;
	for (i = 0; i < m; i++) {
		alpha_to[i] = mask;
		index_of[alpha_to[i]] = i;
		if (p[i] != 0)
			alpha_to[m] ^= mask;
		mask <<= 1;
	}
	index_of[alpha_to[m]] = m;
	mask >>= 1;
	for (i = m + 1; i < n; i++) {
		if (alpha_to[i - 1] >= mask)
		  alpha_to[i] = alpha_to[m] ^ ((alpha_to[i - 1] ^ mask) << 1);
		else
		  alpha_to[i] = alpha_to[i - 1] << 1;
		index_of[alpha_to[i]] = i;
	}
	index_of[0] = -1;
}


void 
gen_poly()
/* 
 * Compute generator polynomial of BCH code of length = 48, redundancy = 12
 * (OK, this is not very efficient, but we only do it once, right? :)
 */
{
	register int    ii, jj, ll, kaux;
	int             test, aux, nocycles, root, noterms, rdncy;
	int             cycle[13][7], size[13], min[13], zeros[13];
	/* Generate cycle sets modulo 63 */
	cycle[0][0] = 0; size[0] = 1;
	cycle[1][0] = 1; size[1] = 1;
	jj = 1;			/* cycle set index */
	do {
		/* Generate the jj-th cycle set */
		ii = 0;
		do {
			ii++;
			cycle[jj][ii] = (cycle[jj][ii - 1] * 2) % n;
			size[jj]++;
			aux = (cycle[jj][ii] * 2) % n;
		} while (aux != cycle[jj][0]);
		/* Next cycle set representative */
		ll = 0;
		do {
			ll++;
			test = 0;
			for (ii = 1; ((ii <= jj) && (!test)); ii++)	
			/* Examine previous cycle sets */
			  for (kaux = 0; ((kaux < size[ii]) && (!test)); kaux++)
					if (ll == cycle[ii][kaux])
						test = 1;
		} while ((test) && (ll < (n - 1)));
		if (!(test)) {
			jj++;	/* next cycle set index */
			cycle[jj][0] = ll;
			size[jj] = 1;
		}
	} while (ll < (n - 1));
	nocycles = jj;		/* number of cycle sets modulo n */
	/* Search for roots 1, 2, ..., d-1 in cycle sets */
	kaux = 0;
	rdncy = 0;
	for (ii = 1; ii <= nocycles; ii++) {
		min[kaux] = 0;
		for (jj = 0; jj < size[ii]; jj++)
			for (root = 1; root < d; root++)
				if (root == cycle[ii][jj])
					min[kaux] = ii;
		if (min[kaux]) {
			rdncy += size[min[kaux]];
			kaux++;
		}
	}
	noterms = kaux;
	kaux = 1;
	for (ii = 0; ii < noterms; ii++)
		for (jj = 0; jj < size[min[ii]]; jj++) {
			zeros[kaux] = cycle[min[ii]][jj];
			kaux++;
		}
	printf("This is a (%d, %d, %d) binary BCH code\n", length, k, d);
	/* Compute generator polynomial */
	g[0] = alpha_to[zeros[1]];
	g[1] = 1;		/* g(x) = (X + zeros[1]) initially */
	for (ii = 2; ii <= rdncy; ii++) {
	  g[ii] = 1;
	  for (jj = ii - 1; jj > 0; jj--)
	    if (g[jj] != 0)
	      g[jj] = g[jj - 1] ^ alpha_to[(index_of[g[jj]] + zeros[ii]) % n];
	    else
	      g[jj] = g[jj - 1];
	  g[0] = alpha_to[(index_of[g[0]] + zeros[ii]) % n];
	}
	printf("g(x) = ");
	for (ii = 0; ii <= rdncy; ii++) {
	  printf("%d", g[ii]);
	  if (ii && ((ii % 70) == 0))
	    printf("\n");
	}
	printf("\n");
}


void 
encode_bch()
/* 
 * Calculate redundant bits bb[], codeword is c(X) = data(X)*X**(n-k)+ bb(X)
 */
{
	register int    i, j;
	register int    feedback;
	for (i = 0; i < length - k; i++)
		bb[i] = 0;
	for (i = k - 1; i >= 0; i--) {
		feedback = data[i] ^ bb[length - k - 1];
		if (feedback != 0) {
			for (j = length - k - 1; j > 0; j--)
				if (g[j] != 0)
					bb[j] = bb[j - 1] ^ feedback;
				else
					bb[j] = bb[j - 1];
			bb[0] = g[0] && feedback;
		} else {
			for (j = length - k - 1; j > 0; j--)
				bb[j] = bb[j - 1];
			bb[0] = 0;
		};
	};
};


void 
decode_bch()
/*
 * We do not need the Berlekamp algorithm to decode.
 * We solve before hand two equations in two variables.
 */
{
	register int    i, j, q;
	int             elp[3], s[5], s3;
	int             count = 0, syn_error = 0;
	int             loc[3], err[3], reg[3];
	int				aux;
	/* first form the syndromes */
	printf("s[] = (");
	for (i = 1; i <= 4; i++) {
		s[i] = 0;
		for (j = 0; j < length; j++)
			if (recd[j] != 0)
				s[i] ^= alpha_to[(i * j) % n];
		if (s[i] != 0)
			syn_error = 1;	/* set flag if non-zero syndrome */
							/* NOTE: If only error detection is needed,
							 * then exit the program here...
							 */
		/* convert syndrome from polynomial form to index form  */
		s[i] = index_of[s[i]];
		printf("%3d ", s[i]);
	};
	printf(")\n");
	if (syn_error) {	/* If there are errors, try to correct them */
		if (s[1] != -1) {
			s3 = (s[1] * 3) % n;
			if ( s[3] == s3 )  /* Was it a single error ? */
				{
				printf("One error at %d\n", s[1]);
				recd[s[1]] ^= 1;		/* Yes: Correct it */
				}
			else {				/* Assume two errors occurred and solve
								 * for the coefficients of sigma(x), the
								 * error locator polynomail
								 */
                if (s[3] != -1)
                  aux = alpha_to[s3] ^ alpha_to[s[3]];
                else
                  aux = alpha_to[s3];

				elp[0] = 0;
				elp[1] = (s[2] - index_of[aux] + n) % n;
				elp[2] = (s[1] - index_of[aux] + n) % n;
				printf("sigma(x) = ");
				for (i = 0; i <= 2; i++)
					printf("%3d ", elp[i]);
				printf("\n");
				printf("Roots: ");
				/* find roots of the error location polynomial */
				for (i = 1; i <= 2; i++)
					reg[i] = elp[i];
				count = 0;
				for (i = 1; i <= 63; i++) { /* Chien search */
					q = 1;
					for (j = 1; j <= 2; j++)
						if (reg[j] != -1) {
							reg[j] = (reg[j] + j) % n;
							q ^= alpha_to[reg[j]];
						}
					if (!q) {	/* store error location number indices */
						loc[count] = i % n;
						count++;
						printf("%3d ", (i%n));
					}
				}
				printf("\n");
				if (count == 2)	
				/* no. roots = degree of elp hence 2 errors */
					for (i = 0; i < 2; i++)
						recd[loc[i]] ^= 1;
				else	/* Cannot solve: Error detection */
					printf("incomplete decoding\n");
				}
			}
		else if (s[2] != -1) /* Error detection */
			printf("incomplete decoding\n");
	}
}


main()
{
	int             i;
	read_p();				/* read generator polynomial g(x) */
	generate_gf();			/* generate the Galois Field GF(2**m) */
	gen_poly();				/* Compute the generator polynomial of BCH code */

	seed = 1;
	srandom(seed);
	/* Randomly generate DATA */
	for (i = 0; i < k; i++)
		data[i] = (random() & 67108864) >> 26;

	/* ENCODE */
	encode_bch();			/* encode data */
 
	for (i = 0; i < length - k; i++)
		recd[i] = bb[i];	/* first (length-k) bits are redundancy */
	for (i = 0; i < k; i++)
		recd[i + length - k] = data[i];	/* last k bits are data */
	printf("c(x) = ");
	for (i = 0; i < length; i++) {
		printf("%1d", recd[i]);
		if (i && ((i % 70) == 0))
			printf("\n");
	}
	printf("\n");

	/* ERRORS */
    printf("Enter the number of errors and their positions: ");
    scanf("%d", &numerr);
	for (i = 0; i < numerr; i++)
		{
		scanf("%d", &errpos[i]);
		recd[errpos[i]] ^= 1;
		}
	printf("r(x) = ");
	for (i = 0; i < length; i++)
		printf("%1d", recd[i]);
	printf("\n");

    /* DECODE */
	decode_bch();
	/*
	 * print out original and decoded data
	 */
	printf("Results:\n");
	printf("original data  = ");
	for (i = 0; i < k; i++)
		printf("%1d", data[i]);
	printf("\nrecovered data = ");
	for (i = length - k; i < length; i++)
		printf("%1d", recd[i]);
	printf("\n");
	/* decoding errors: we compare only the data portion */
	for (i = length - k; i < length; i++)
		if (data[i - length + k] != recd[i])
			decerror++;
	if (decerror)
		printf("%d message decoding errors\n", decerror);
	else
		printf("Succesful decoding\n");
}
