
/*
 * Overview:
 *   Generic Reed Solomon encoder / decoder library
 *
 * Copyright 2002, Phil Karn, KA9Q
 * May be used under the terms of the GNU General Public License (GPL)
 *
 * Adaption to the kernel by Thomas Gleixner (tglx@linutronix.de)
 * Converted to C++ by Perry Kundert (perry@hardconsulting.com)
 * 
 *     The Linux 3.15.1 version of lib/reed_solomon was used, which (in turn) is basically verbatim
 * copied from Phil Karn's implementation.  I've personally been using Phil's implementation for
 * years in a heavy industrial use, and it is rock-solid.
 * 
 */
#ifndef _EZPWD_RS
#define _EZPWD_RS

#include <algorithm>
#include <vector>
#include <cstdint>
#include <iostream>
#include <iomanip>

// ARRAY_SAFE -- define to force usage of bounds-checked arrays for most tabular data
// ARRAY_TEST -- define to force erroneous sizing of some arrays
#if defined( ARRAY_SAFE )
#  include <array_safe>
#else
#  include <array>
#  define array_safe std::array
#endif

namespace ezpwd {

    /**
     * reed_solomon_base - Reed-Solomon codec generic base class
     */
    class reed_solomon_base {
    public:
	virtual int		datum()		const = 0;	// a data element's bits
	virtual int		symbol()	const = 0;	// a symbol's bits
	virtual int		size()		const = 0;	// R-S block size
	virtual int		nroots()	const = 0;	// R-S roots
	virtual	int		load()		const = 0;	// R-S net payload

	virtual		       ~reed_solomon_base()
	{
	    ;
	}
				reed_solomon_base()
	{
	    ;
	}

	virtual void		encode(
				    std::string	       &data,
				    std::string	       *parity	= 0 )
	    const
	= 0;

	// 
	// {en,de}code -- Compute/Correct errors/erasures in a Reed-Solomon encoded std::string
	// 
	///     The parity symbols may be included in 'data', or may (optionally) supplied
	/// separately in (at least nroots-sized) 'parity'.  Optionally specify some known erasure
	/// positions.  If 'erasures' is specified, it wil be extended to be capable of storing up
	/// to 'nroots()' ints; the actual deduced error locations will be returned.
	///  
	/// RETURN VALUE
	/// 
	///     The number of symbols corrected.  Both errors and erasures are included, so long as
	/// they are actually different than the deduced value.  In other words, if a symbol is
	/// marked as an erasure but it actually turns out to be correct, it's index will NOT be
	/// included in the returned count, or the modified *era array!
	/// 
	virtual
	int			decode(
				    std::string	       &data,
				    std::string	       *parity	= 0,
				    std::vector<int>   *erasure	= 0 )
	    const
	= 0;
    };

    /**
     * gfpoly - default field polynomial generator functor.
     */
    template < int SYM, int PLY >
    struct gfpoly {
	int 			operator() ( int sr )
	    const
	{
	    if ( sr == 0 )
		sr			= 1;
	    else {
		sr	      	      <<= 1;
		if ( sr & ( 1 << SYM ))
		    sr	       	       ^= PLY;
		sr		       &= (( 1 << SYM ) - 1);
	    }
	    return sr;
	}
    };

    /**
     * struct reed_solomon - Reed-Solomon codec
     *
     * @MM:		Bits per symbol
     * @NN:		Symbols per block (= (1<<mm)-1)
     * @alpha_to:	log lookup table
     * @index_of:	Antilog lookup table
     * @genpoly:	Generator polynomial
     * @nroots:		Number of generator roots = number of parity symbols
     * @fcr:		First consecutive root, index form
     * @prim:		Primitive element, index form
     * @iprim:		prim-th root of 1, index form
     * @poly:		The primitive generator polynominal functor
     * @mutex_t:	A std::mutex like object, or a dummy
     * @guard_t:	A std::lock_guard, or anything that can take a mutex_t
     *
     *     All reed_solomon<T, ...> instances with the same template type parameters share a common
     * (static) set of alpha_to, index_of and genpoly tables.  The first instance to be constructed
     * initializes the tables (optionally protected by a std::mutex/std::lock_guard).
     * 
     *     Each specialized type of reed_solomon implements a specific encode/decode method
     * appropriate to its data 'TYP' 'data_t'.  When accessed via a generic reed_solomon_base
     * pointer, 
     */
    template < typename TYP, int SYM, int RTS, int FCR, int PRM, class PLY,
	       typename MTX=int, typename GRD=int >
    class reed_solomon
	: public reed_solomon_base {
    public:
	typedef TYP		data_t;
	typedef MTX		mutex_t;
	typedef GRD		guard_t;

	static const int	DATA	= 8 * sizeof data_t();	// bits / data_t
	static const int	SYMBOL	= SYM;			// bits / symbol
	static const int	NROOTS	= RTS;
	static const int	SIZE	= ( 1 << SYMBOL ) - 1;
	static const int	LOAD	= SIZE - NROOTS;
	static const int	MM	= SYMBOL;
	static const int	NN	= SIZE;
	static const int	A0	= NN;

	virtual int		datum() const
	{
	    return DATA;
	}

	virtual int		symbol() const
	{
	    return SYMBOL;
	}

	virtual int		size() const
	{
	    return SIZE;
	}

	virtual int		nroots() const
	{
	    return NROOTS;
	}

	virtual int		load() const
	{
	    return LOAD;
	}

	using reed_solomon_base::encode;
	virtual void		encode(
				    std::string	       &data,
				    std::string	       *parity	= 0 )
	    const
	{
	    if ( parity ) {
		parity->resize( NROOTS );
		encode( (data_t *)&data.front(), data.size(),
			(data_t *)&parity->front() );
	    } else {
		data.resize( data.size() + NROOTS );
		encode( (data_t *)&data.front(), data.size() - NROOTS,
			(data_t *)&data.front() + data.size() - NROOTS );
	    }
	}
	
	using reed_solomon_base::decode;
	virtual	int		decode(
				    std::string	       &data,
				    std::string	       *parity	= 0,
				    std::vector<int>   *erasure	= 0 )	// positions
	    const
	{
	    uint8_t		       *dataptr	= (uint8_t *)&data.front();
	    int				datalen	= data.size();
	    uint8_t		       *pariptr;
	    if ( parity ) {
		if ( parity->size() < NROOTS )
		    throw std::runtime_error( "reed-solomon: parity std::string::size incompatible with number of roots" );
		pariptr				= (uint8_t *)&parity->front();
	    } else {
		datalen			       -= NROOTS;
		pariptr				= dataptr + datalen;
	    }
	    if ( ! erasure )
		return decode( dataptr, datalen, pariptr );
	    int			erasures	= erasure->size();
	    erasure->resize( NROOTS );
	    int			corrects	= decode( dataptr, datalen, pariptr,
							  &erasure->front(), erasures );
	    erasure->resize( std::max( 0, corrects ));
	    return corrects;
	}
	
    protected:
	static mutex_t		mutex;
	static int 		iprim;

#if defined( ARRAY_TEST )
#  warning "ARRAY_TEST: Erroneously declaring alpha_to size!"
	static array_safe<data_t,NN    >
#else
	static array_safe<data_t,NN + 1>
#endif
				alpha_to;
	static array_safe<data_t,NN + 1>
				index_of;
	static array_safe<data_t,NROOTS + 1>
				genpoly;

	/** modulo replacement for galois field arithmetics
	 *
	 *  @x:		the value to reduce
	 *
	 *  where
	 *  MM = number of bits per symbol
	 *  NN = (2^MM) - 1
	 *
	 *  Simple arithmetic modulo would return a wrong result for values >= 3 * NN
	 */
	int	 		modnn(
				    int 		x )
	    const
	{
	    while ( x >= NN ) {
		x		       -= NN;
		x			= ( x >> MM ) + ( x & NN );
	    }
	    return x;
	}

    public:
	virtual		       ~reed_solomon()
	{
	    ;
	}
				reed_solomon()
				    : reed_solomon_base()
	{
	    // lock, if guard/mutex provided, and do init if not already done
	    guard_t		guard( mutex ); (void)guard;
	    if ( iprim )
		return;

	    /* Generate Galois field lookup tables */
	    index_of[0]			= A0;	/* log(zero) = -inf */
	    alpha_to[A0]		= 0;	/* alpha**-inf = 0 */
	    PLY			poly;
	    int			sr	= poly( 0 );
	    for ( int i = 0; i < NN; i++ ) {
		index_of[sr]		= i;
		alpha_to[i]		= sr;
		sr			= poly( sr );
	    }
	    /* If it's not primitive, exit */
	    if ( sr != alpha_to[0] )
		throw std::runtime_error( "reed-solomon: Galois field polynomial not primitive" );

	    /* Find prim-th root of 1, used in decoding */
	    for ( iprim = 1; (iprim % PRM) != 0; iprim += NN )
		;
	    /* prim-th root of 1, index form */
	    iprim 		       /= PRM;

	    /* Form RS code generator polynomial from its roots */
	    genpoly[0]			= 1;
	    for ( int i = 0, root = FCR * PRM; i < NROOTS; i++, root += PRM ) {
		genpoly[i + 1]		= 1;
		/* Multiply genpoly[] by  @**(root + x) */
		for ( int j = i; j > 0; j-- ) {
		    if ( genpoly[j] != 0 )
			genpoly[j]	= genpoly[j - 1]
			    ^ alpha_to[modnn(index_of[genpoly[j]] + root)];
		    else
			genpoly[j]	= genpoly[j - 1];
		}
		/* genpoly[0] can never be zero */
		genpoly[0]		= alpha_to[modnn(index_of[genpoly[0]] + root)];
	    }
	    /* convert genpoly[] to index form for quicker encoding */
	    for ( int i = 0; i <= NROOTS; i++ )
		genpoly[i]		= index_of[genpoly[i]];
	}

	void			encode(
				    const data_t       *data,
				    int			len,
				    data_t	       *parity, // at least nroots
				    data_t		invmsk	= 0 )
	    const
	{
	    /* Check length parameter for validity */
	    int			pad	= NN - NROOTS - len;
	    if ( pad < 0 || pad >= NN )
		throw std::runtime_error( "reed-solomon: data length incompatible with block size and error correction symbols" );
	    for ( int i = 0; i < NROOTS; i++ )
		parity[i]		= 0;
	    for ( int i = 0; i < len; i++ ) {
		data_t		feedback= index_of[data[i] ^ invmsk ^ parity[0]];
		if ( feedback != A0 ) /* feedback term is non-zero */
		    for ( int j = 1; j < NROOTS; j++ )
			parity[j]       ^= alpha_to[modnn(feedback + genpoly[NROOTS - j])];

		/* Shift */
		// was: memmove( &par[0], &par[1], ( sizeof par[0] ) * ( NROOTS - 1 ));
		std::rotate( parity, parity + 1, parity + NROOTS );
		if ( feedback != A0 ) {
		    parity[NROOTS - 1]	= alpha_to[modnn(feedback + genpoly[0])];
		} else {
		    parity[NROOTS - 1]	= 0;
		}
	    }
	}

	int			decode(
				    data_t	       *data,
				    int			len,
				    data_t	       *parity,
				    int		       *eras_pos= 0,
				    int			no_eras	= 0,
				    data_t	       *corr	= 0,
				    data_t		invmsk	= 0 )
	    const
	{
	    typedef array_safe< data_t, NROOTS >
				data_nroots;
	    typedef array_safe< data_t, NROOTS+1 >
				data_nroots_1;
	    typedef array_safe< int, NROOTS >
				ints_nroots;

	    data_nroots_1	lambda { { { 0 } } };
	    data_nroots		syn;
	    data_nroots_1	b;
	    data_nroots_1	t;
	    data_nroots_1	omega;
	    ints_nroots		root;
	    data_nroots_1	reg;
	    ints_nroots		loc;
	    int			count	= 0;

	    /* Check length parameter for validity */
	    int			pad	= NN - NROOTS - len;
	    if ( pad < 0 || pad >= NN )
		throw std::runtime_error( "reed-solomon: data length incompatible with block size and error correction symbols" );
	    if ( no_eras )
		for ( int i = 0; i < no_eras; ++i )
		    if ( eras_pos[i] < 0 || eras_pos[i] >= len + NROOTS )
			throw std::runtime_error( "reed-solomon: erasure positions outside data+parity" );

	    // form the syndromes; i.e., evaluate data(x) at roots of g(x)
	    for ( int i = 0; i < NROOTS; i++ )
		syn[i]			= data[0] ^ invmsk;

	    for ( int j = 1; j < len; j++ ) {
		for ( int i = 0; i < NROOTS; i++ ) {
		    if ( syn[i] == 0 ) {
			syn[i]		= data[j] ^ invmsk;
		    } else {
			syn[i]		= data[j] ^ invmsk
			    ^ alpha_to[modnn(index_of[syn[i]] + ( FCR + i ) * PRM)];
		    }
		}
	    }

	    for ( int j = 0; j < NROOTS; j++ ) {
		for ( int i = 0; i < NROOTS; i++ ) {
		    if ( syn[i] == 0 ) {
			syn[i]		= parity[j];
		    } else {
			syn[i] 		= parity[j]
			    ^ alpha_to[modnn(index_of[syn[i]] + ( FCR + i ) * PRM)];
		    }
		}
	    }

	    // Convert syndromes to index form, checking for nonzero condition
	    data_t 		syn_error = 0;
	    for ( int i = 0; i < NROOTS; i++ ) {
		syn_error	       |= syn[i];
		syn[i]			= index_of[syn[i]];
	    }

	    int			deg_lambda = 0;
	    int			deg_omega = 0;
	    int			r	= no_eras;
	    int			el	= no_eras;
	    if (!syn_error) {
		// if syndrome is zero, data[] is a codeword and there are no errors to correct.
		count			= 0;
		goto finish;
	    }

	    lambda[0] 			= 1;
	    if ( no_eras > 0 ) {
		// Init lambda to be the erasure locator polynomial.  Convert erasure positions
		// from index into data, to index into Reed-Solomon block.
		lambda[1]		= alpha_to[modnn(PRM * (NN - 1 - ( eras_pos[0] + pad )))];
		for ( int i = 1; i < no_eras; i++ ) {
		    data_t	u	= modnn(PRM * (NN - 1 - ( eras_pos[i] + pad )));
		    for ( int j = i + 1; j > 0; j-- ) {
			data_t	tmp	= index_of[lambda[j - 1]];
			if ( tmp != A0 ) {
			    lambda[j]  ^= alpha_to[modnn(u + tmp)];
			}
		    }
		}
	    }

#if DEBUG >= 1
	    // Test code that verifies the erasure locator polynomial just constructed
	    // Needed only for decoder debugging.
    
	    // find roots of the erasure location polynomial
	    for( int i = 1; i<= no_eras; i++ )
		reg[i]			= index_of[lambda[i]];

	    count			= 0;
	    for ( int i = 1, k = iprim - 1; i <= NN; i++, k = modnn( k + iprim )) {
		data_t		q	= 1;
		for ( int j = 1; j <= no_eras; j++ ) {
		    if ( reg[j] != A0 ) {
			reg[j]		= modnn( reg[j] + j );
			q	       ^= alpha_to[reg[j]];
		    }
		}
		if ( q != 0 )
		    continue;
		// store root and error location number indices
		root[count]		= i;
		loc[count]		= k;
		count++;
	    }
	    if ( count != no_eras ) {
		std::cout << "ERROR: count = " << count << ", no_eras = " << no_eras 
			  << "lambda(x) is WRONG"
			  << std::endl;
		count = -1;
		goto finish;
	    }
#if DEBUG >= 2
	    if ( count ) {
	        std::cout
		    << "Erasure positions as determined by roots of Eras Loc Poly: ";
		for ( int i = 0; i < count; i++ )
		    std::cout << loc[i] << ' ';
		std::cout << std::endl;
	        std::cout
		    << "Erasure positions as determined by roots of eras_pos array: ";
		for ( int i = 0; i < no_eras; i++ )
		    std::cout << eras_pos[i] << ' ';
		std::cout << std::endl;
	    }
#endif
#endif

	    for ( int i = 0; i < NROOTS + 1; i++ )
		b[i]			= index_of[lambda[i]];

	    //
	    // Begin Berlekamp-Massey algorithm to determine error+erasure locator polynomial
	    //
	    while ( ++r <= NROOTS ) { // r is the step number
		// Compute discrepancy at the r-th step in poly-form
		data_t		discr_r	= 0;
		for ( int i = 0; i < r; i++ ) {
		    if (( lambda[i] != 0 ) && ( syn[r - i - 1] != A0 )) {
			discr_r	       ^= alpha_to[modnn(index_of[lambda[i]] + syn[r - i - 1])];
		    }
		}
		discr_r			= index_of[discr_r];	/* Index form */
		if ( discr_r == A0 ) {
		    // 2 lines below: B(x) <-- x*B(x)
		    // Rotate the last element of b[NROOTS+1] to b[0]
		    std::rotate( b.begin(), b.begin()+NROOTS, b.end() );
		    b[0]		= A0;
		} else {
		    // 7 lines below: T(x) <-- lambda(x)-discr_r*x*b(x)
		    t[0]		= lambda[0];
		    for ( int i = 0; i < NROOTS; i++ ) {
			if ( b[i] != A0 ) {
			    t[i + 1]	= lambda[i + 1]
				^ alpha_to[modnn(discr_r + b[i])];
			} else
			    t[i + 1]	 = lambda[i + 1];
		    }
		    if ( 2 * el <= r + no_eras - 1 ) {
			el		= r + no_eras - el;
			//2 lines below: B(x) <-- inv(discr_r) * lambda(x)
			for ( int i = 0; i <= NROOTS; i++ ) {
			    b[i]	= ((lambda[i] == 0)
					   ? A0
					   : modnn(index_of[lambda[i]] - discr_r + NN));
			}
		    } else {
			// 2 lines below: B(x) <-- x*B(x)
			std::rotate( b.begin(), b.begin()+NROOTS, b.end() );
			b[0]		= A0;
		    }
		    lambda		= t;
		}
	    }

	    // Convert lambda to index form and compute deg(lambda(x))
	    for ( int i = 0; i < NROOTS + 1; i++ ) {
		lambda[i]		= index_of[lambda[i]];
		if ( lambda[i] != NN )
		    deg_lambda		= i;
	    }
	    // Find roots of error+erasure locator polynomial by Chien search
	    reg				= lambda;
	    count			= 0; // Number of roots of lambda(x)
	    for ( int i = 1, k = iprim - 1; i <= NN; i++, k = modnn( k + iprim )) {
		data_t		q	= 1; // lambda[0] is always 0
		for ( int j = deg_lambda; j > 0; j-- ) {
		    if ( reg[j] != A0 ) {
			reg[j]		= modnn( reg[j] + j );
			q	       ^= alpha_to[reg[j]];
		    }
		}
		if ( q != 0 )
		    continue; // Not a root
		// store root (index-form) and error location number
#if DEBUG>=2
		std::cout << "count " << count << " root " << i << " loc " << k << std::endl;
#endif
		root[count]		= i;
		loc[count]		= k;
		// If we've already found max possible roots, abort the search to save time
		if ( ++count == deg_lambda )
		    break;
	    }
	    if ( deg_lambda != count ) {
		// deg(lambda) unequal to number of roots => uncorrectable error detected
		count			= -1;
		goto finish;
	    }
	    /*
	     * Compute err+eras evaluator poly omega(x) = s(x)*lambda(x) (modulo x**NROOTS). in
	     * index form. Also find deg(omega).
	     */
	    deg_omega 			= deg_lambda - 1;
	    for ( int i = 0; i <= deg_omega; i++ ) {
		data_t		tmp	= 0;
		for ( int j = i; j >= 0; j-- ) {
		    if (( syn[i - j] != A0 ) && ( lambda[j] != A0 ))
			tmp	       ^= alpha_to[modnn(syn[i - j] + lambda[j])];
		}
		omega[i]		= index_of[tmp];
	    }

	    /*
	     * Compute error values in poly-form. num1 = omega(inv(X(l))), num2 = inv(X(l))**(fcr-1)
	     * and den = lambda_pr(inv(X(l))) all in poly-form
	     */
	    for ( int j = count - 1; j >= 0; j-- ) {
		data_t		num1	= 0;
		for ( int i = deg_omega; i >= 0; i-- ) {
		    if ( omega[i] != A0 )
			num1	       ^= alpha_to[modnn(omega[i] + i * root[j])];
		}
		data_t		num2	= alpha_to[modnn(root[j] * ( FCR - 1 ) + NN)];
		data_t		den	= 0;

		// lambda[i+1] for i even is the formal derivative lambda_pr of lambda[i]
		for ( int i = std::min(deg_lambda, NROOTS - 1) & ~1; i >= 0; i -= 2 ) {
		    if ( lambda[i + 1] != A0 ) {
			den	       ^= alpha_to[modnn(lambda[i + 1] + i * root[j])];
		    }
		}
#if DEBUG >= 1
		if (den == 0) {
		    std::cout << "ERROR: denominator = 0" << std::endl;
		    count = -1;
		    goto finish;
		}
#endif
		// Apply error to data
		if ( num1 != 0 && loc[j] >= pad ) {
		    data_t	cor	= alpha_to[modnn(index_of[num1]
							 + index_of[num2]
							 + NN - index_of[den])];
		    // Store the error correction pattern, if a correction buffer is available
		    if ( corr )
			corr[j] 	= cor;
		    // If a data/parity buffer is given and the error is inside the message or
		    // parity data, correct it
		    if ( loc[j] < ( NN - NROOTS )) {
			if ( data ) {
			    data[loc[j] - pad] ^= cor;
			}
		    } else if ( loc[j] < NN ) {
		        if ( parity )
		            parity[loc[j] - ( NN - NROOTS )] ^= cor;
		    }
		}
	    }

	finish:
	    if ( eras_pos != NULL ) {
		for ( int i = 0; i < count; i++)
		    eras_pos[i]		= loc[i] - pad;
	    }
	    return count;
	}
    }; // class reed_solomon

    // 
    // Define the static members; allowed in header for template types.
    // 
    //     The reed_solomon<...>::iprim == 0 is used to indicate to the first instance that the
    // static tables require initialization.  If reed_solomon<...>::mutex is something like a
    // std::mutex, and guard_t is a std::lock_guard, then the mutex is acquired for the test and
    // initialization.
    // 
    template < typename TYP, int SYM, int RTS, int FCR, int PRM, class PLY, typename MTX, typename GRD >
        MTX				reed_solomon< TYP, SYM, RTS, FCR, PRM, PLY, MTX, GRD >::mutex;
    template < typename TYP, int SYM, int RTS, int FCR, int PRM, class PLY, typename MTX, typename GRD >
        int				reed_solomon< TYP, SYM, RTS, FCR, PRM, PLY, MTX, GRD >::iprim = 0;
    template < typename TYP, int SYM, int RTS, int FCR, int PRM, class PLY, typename MTX, typename GRD >
#if defined( ARRAY_TEST )
#  warning "ARRAY_TEST: Erroneously defining alpha_to size!"
        array_safe< TYP, reed_solomon< TYP, SYM, RTS, FCR, PRM, PLY, MTX, GRD >::NN     >
#else
        array_safe< TYP, reed_solomon< TYP, SYM, RTS, FCR, PRM, PLY, MTX, GRD >::NN + 1 >
#endif
					reed_solomon< TYP, SYM, RTS, FCR, PRM, PLY, MTX, GRD >::alpha_to;
    template < typename TYP, int SYM, int RTS, int FCR, int PRM, class PLY, typename MTX, typename GRD >
        array_safe< TYP, reed_solomon< TYP, SYM, RTS, FCR, PRM, PLY, MTX, GRD >::NN + 1 >
					reed_solomon< TYP, SYM, RTS, FCR, PRM, PLY, MTX, GRD >::index_of;
    template < typename TYP, int SYM, int RTS, int FCR, int PRM, class PLY, typename MTX, typename GRD >
        array_safe< TYP, reed_solomon< TYP, SYM, RTS, FCR, PRM, PLY, MTX, GRD >::NROOTS + 1 >
					reed_solomon< TYP, SYM, RTS, FCR, PRM, PLY, MTX, GRD >::genpoly;

    // ezpwd::log_<N,B> -- compute the log base B of N at compile-time
    template <size_t N, size_t B=2> struct log_{       enum { value = 1 + log_<N/B, B>::value }; };
    template <size_t B>		    struct log_<1, B>{ enum { value = 0 }; };
    template <size_t B>		    struct log_<0, B>{ enum { value = 0 }; };

    // 
    // RS( ... ) -- Define a reed-solomon codec 
    // 
    // @SYMBOLS:	Total number of symbols; must be a power of 2 minus 1, eg 2^8-1 == 255
    // @PAYLOAD:	The maximum number of non-parity symbols, eg 253 ==> 2 parity symbols
    // @POLY:		A primitive polynomial appropriate to the SYMBOLS size
    // @FCR:		The first consecutive root of the Reed-Solomon generator polynomial
    // @PRIM:		The primitive root of the generator polynomial
    // 
#   define RS( TYPE, SYMBOLS, PAYLOAD, POLY, FCR, PRIM )\
	ezpwd::reed_solomon<				\
	    uint8_t,					\
	    ezpwd::log_< (SYMBOLS)+1 >::value,		\
	    (SYMBOLS)-(PAYLOAD),			\
	    FCR,					\
	    PRIM,					\
	    ezpwd::gfpoly<				\
		ezpwd::log_< (SYMBOLS)+1 >::value,	\
		    POLY >>

    //
    // RS_<SYMBOLS>( PAYLOAD ) -- Standard Reed-Solomon codec type access
    //
    // Normally, Reed-Solomon codecs are described with terms like RS(255,252).	 Obtain various
    // standard Reed-Solomon codecs using macros of a similar form, eg. RS_255( 252 ).	Standard
    // POLY, FCR and PRIM values are provided for various SYMBOL sizes, along with appropriate basic
    // types capable of holding all internal Reed-Solomon tabular data.
    // 
#   define RS_3( PAYLOAD )		RS( uint8_t,	  3, PAYLOAD,	  0x7,	 1,  1 )
#   define RS_7( PAYLOAD )		RS( uint8_t,	  7, PAYLOAD,	  0xb,	 1,  1 )
#   define RS_15( PAYLOAD )		RS( uint8_t,	 15, PAYLOAD,	 0x13,	 1,  1 )
#   define RS_31( PAYLOAD )		RS( uint8_t,	 31, PAYLOAD,	 0x25,	 1,  1 )
#   define RS_63( PAYLOAD )		RS( uint8_t,	 63, PAYLOAD,	 0x43,	 1,  1 )
#   define RS_127( PAYLOAD )		RS( uint8_t,	127, PAYLOAD,	 0x89,	 1,  1 )
#   define RS_255( PAYLOAD )		RS( uint8_t,	255, PAYLOAD,	0x11d,	 1,  1 )
#   define RS_255_CCSDS( PAYLOAD )	RS( uint8_t,	255, PAYLOAD,	0x187, 112, 11 )
#   define RS_511( PAYLOAD )		RS( uint16_t,	511, PAYLOAD,	0x211,	 1,  1 )
#   define RS_1023( PAYLOAD )		RS( uint16_t,  1023, PAYLOAD,	0x409,	 1,  1 )
#   define RS_2047( PAYLOAD )		RS( uint16_t,  2047, PAYLOAD,	0x805,	 1,  1 )
#   define RS_4095( PAYLOAD )		RS( uint16_t,  4095, PAYLOAD,  0x1053,	 1,  1 )
#   define RS_8191( PAYLOAD )		RS( uint16_t,  8191, PAYLOAD,  0x201b,	 1,  1 )
#   define RS_16383( PAYLOAD )		RS( uint16_t, 16383, PAYLOAD,  0x4443,	 1,  1 )
#   define RS_32767( PAYLOAD )		RS( uint16_t, 32767, PAYLOAD,  0x8003,	 1,  1 )
#   define RS_65535( PAYLOAD )		RS( uint16_t, 65535, PAYLOAD, 0x1100b,	 1,  1 )

} // namespace ezpwd

// 
// std::ostream << ezpwd::reed_solomon<...>
//
inline
std::ostream		       &operator<<(
				    std::ostream       &lhs,
				    const ezpwd::reed_solomon_base
						       &rhs )
{
    return lhs << "RS(" << rhs.size() << "," << rhs.load() << ")";
}

struct hexify {
    unsigned char		c;
    std::streamsize		w;
				hexify(
				    unsigned char	_c,
				    std::streamsize	_w	= 2 )
				    : c( _c )
				    , w( _w )
    { ; }
				hexify(
				    char		_c,
				    std::streamsize	_w	= 2 )
				    : c( (unsigned char)_c )
				    , w( _w )
    { ; }
};

inline
std::ostream		       &operator<<(
				    std::ostream       &lhs,
				    const hexify       &rhs )
{
    std::ios_base::fmtflags	flg	= lhs.flags();			// left, right, hex?
    
    lhs << std::setw( rhs.w );
    if ( isprint( rhs.c ) || isspace( rhs.c )) {
	switch ( char( rhs.c )) {
	case 0x00: lhs << "\\0";  break;		// NUL
	case 0x07: lhs << "\\a";  break;		// BEL
	case 0x08: lhs << "\\b";  break;		// BS
	case 0x1B: lhs << "\\e";  break;		// ESC
	case 0x09: lhs << "\\t";  break;		// HT
	case 0x0A: lhs << "\\n";  break;		// LF
	case 0x0B: lhs << "\\v";  break;		// VT
	case 0x0C: lhs << "\\f";  break;		// FF
	case 0x0D: lhs << "\\r";  break;		// CR
	case ' ':  lhs << "  ";   break;		// space
	case '\\': lhs << "\\\\"; break;		// '\'
	default:   lhs << char( rhs.c );		// any other printable character
	}
    } else {
	char			fill	= lhs.fill();
	lhs << std::setfill( '0' ) << std::hex << std::uppercase 
	    << (unsigned int)rhs.c
	    << std::setfill( fill ) << std::dec << std::nouppercase;
    }
    lhs.flags( flg );
    return lhs;
}


template < typename iter_t >
inline
std::ostream		       &hexout(
				    std::ostream       &lhs,
				    const iter_t       &beg,
				    const iter_t       &end )
{
    int				col	= 0;
    for ( auto i = beg; i < end; ++i ) {
	if ( col == 40 ) {
	    lhs << std::endl;
	    col				= 0;
	}
	lhs << hexify( *i );
	++col;
	if ( *i == '\n' )
	    col				= 40;
    }
    return lhs;
}
				    
template <unsigned char, int N>
inline
std::ostream		       &operator<<(
				    std::ostream       &lhs,
				    const std::array<unsigned char,N>
						       &rhs )
{
    return hexout( lhs, rhs.begin(), rhs.end() );
}

inline
std::ostream		       &operator<<(
				    std::ostream       &lhs,
				    const std::vector<unsigned char>
						       &rhs )
{
    return hexout( lhs, rhs.begin(), rhs.end() );
}
    
#endif // _EZPWD_RS
