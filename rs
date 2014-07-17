
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
#include <array>
#include <map>
#include <type_traits>
#include <cstdint>
#include <cstring>
#include <iostream>

#include "hex" // ezpwd::hex... std::ostream shims for outputting containers of uint8_t data

namespace ezpwd {

#if defined( EZPWD_ARRAY_SAFE )
    // 
    // ezpwd::array -- a std::array with bounds checking enabled by default
    // 
    //     Only enabled when EZPWD_ARRAY_SAFE is defined in the preprocessor
    // 
    // EZPWD_ARRAY_SAFE -- define to force usage of bounds-checked arrays for most tabular data
    // EZPWD_ARRAY_TEST -- define to force erroneous sizing of some arrays for non-production testing
    // 
    template < typename T, std::size_t S >
    struct array
	: public std::array<T, S> {
	using typename std::array<T,S>::value_type;
	using typename std::array<T,S>::pointer;
	using typename std::array<T,S>::const_pointer;
	using typename std::array<T,S>::reference;
	using typename std::array<T,S>::const_reference;
	using typename std::array<T,S>::iterator;
	using typename std::array<T,S>::const_iterator;
	using typename std::array<T,S>::size_type;
	using typename std::array<T,S>::difference_type;
	using typename std::array<T,S>::reverse_iterator;
	using typename std::array<T,S>::const_reverse_iterator;

				array()
				    : std::array<T,S>() {;}

				array( const std::array<T,S> &rhs )
				    : std::array<T,S>( rhs ) {;}

				array( const std::array<T,S> &&rhs )
				    : std::array<T,S>( rhs ) {;}

	using std::array<T,S>::fill;
	using std::array<T,S>::swap;
	using std::array<T,S>::begin;
	using std::array<T,S>::end;
	using std::array<T,S>::rbegin;
	using std::array<T,S>::rend;
	using std::array<T,S>::cbegin;
	using std::array<T,S>::cend;
	using std::array<T,S>::crbegin;
	using std::array<T,S>::crend;
	using std::array<T,S>::size;
	using std::array<T,S>::max_size;
	using std::array<T,S>::empty;
	using std::array<T,S>::at;
	using std::array<T,S>::front;
	using std::array<T,S>::back;
	using std::array<T,S>::data;

	reference		operator[]( size_type i )
	{
	    return at( i );
	}

	constexpr
	const_reference		operator[]( size_type i )
	    const
	{
	    return at( i );
	}
    }; // struct ezpwd::array
#else
    using std::array; // ! EZPWD_ARRAY_SAFE: ezpwd::arrray is std::array
#endif

    //
    // reed_solomon_base - Reed-Solomon codec generic base class
    //
    class reed_solomon_base {
    public:
	virtual size_t		datum()		const = 0;	// a data element's bits
	virtual size_t		symbol()	const = 0;	// a symbol's bits
	virtual int		size()		const = 0;	// R-S block size (maximum total symbols)
	virtual int		nroots()	const = 0;	// R-S roots (parity symbols)
	virtual	int		load()		const = 0;	// R-S net payload (data symbols)

	virtual		       ~reed_solomon_base()
	{
	    ;
	}
				reed_solomon_base()
	{
	    ;
	}

	// 
	// {en,de}code -- Compute/Correct errors/erasures in a Reed-Solomon encoded container
	// 
	///     The parity symbols may be included in 'data', or may (optionally) supplied
	/// separately in (at least nroots-sized) 'parity'.  Optionally specify some known erasure
	/// positions.  If 'erasures' is specified, its capacity will be increased to be capable of
	/// storing up to 'nroots()' ints; the actual deduced error locations will be returned.
	///  
	/// RETURN VALUE
	/// 
	///     The number of symbols corrected.  Both errors and erasures are included, so long as
	/// they are actually different than the deduced value.  In other words, if a symbol is
	/// marked as an erasure but it actually turns out to be correct, it's index will NOT be
	/// included in the returned count, nor the modified erasure vector!
	///

	// 
	// encode(<string>) -- extend string to contain parity, or place in supplied parity string
	// encode(<vector>) -- extend vector to contain parity, or place in supplied parity vector
	// encode(<array>)  -- ignore 'pad' elements of array, puts nroots() parity symbols at end
	// 
	void			encode(
				    std::string	       &data )
	    const
	{
	    typedef uint8_t	uT;
	    typedef std::pair<uT *, uT *>
				uTpair;
	    data.resize( data.size() + nroots() );
	    encode( uTpair( (uT *)&data.front(), (uT *)&data.front() + data.size() ));
	}

	void			encode(
				    const std::string  &data,
				    std::string	       &parity )
	    const
	{
	    typedef uint8_t	uT;
	    typedef std::pair<const uT *, const uT *>
				cuTpair;
	    typedef std::pair<uT *, uT *>
				uTpair;
	    parity.resize( nroots() );
	    encode( cuTpair( (uT *)&data.front(), (uT *)&data.front() + data.size() ),
		    uTpair( (uT *)&parity.front(), (uT *)&parity.front() + parity.size() ));
	}

	template < typename T >
	void			encode(
				    std::vector<T>     &data )
	    const
	{
	    typedef typename std::make_unsigned<T>::type
				uT;
	    typedef std::pair<uT *, uT *>
				uTpair;
	    data.resize( data.size() + nroots() );
	    encode( uTpair( (uT *)&data.front(), (uT *)&data.front() + data.size() ));
	}
	template < typename T >
	void			encode(
				    const std::vector<T>&data,
				    std::vector<T>     &parity )
	    const
	{
	    typedef typename std::make_unsigned<T>::type
				uT;
	    typedef std::pair<const uT *, const uT *>
				cuTpair;
	    typedef std::pair<uT *, uT *>
				uTpair;
	    parity.resize( nroots() );
	    encode( cuTpair( (uT *)&data.front(), (uT *)&data.front() + data.size() ),
		    uTpair( (uT *)&parity.front(), (uT *)&parity.front() + parity.size() ));
	}

	template < typename T, size_t N >
	void			encode(
				    std::array<T,N>    &data,
				    int			pad	= 0, // ignore 'pad' symbols at start of array
				    std::vector<int>   *erasure	= 0 )
	    const
	{
	    typedef typename std::make_unsigned<T>::type
				uT;
	    typedef std::pair<uT *, uT *>
				uTpair;
	    encode( uTpair( (uT *)&data.front() + pad, (uT *)&data.front() + data.size() ));
	}

	virtual void		encode(
				    const std::pair<uint8_t *, uint8_t *>
						       &data )
	    const
	= 0;
	virtual void		encode(
				    const std::pair<const uint8_t *, const uint8_t *>
						       &data,
				    const std::pair<uint8_t *, uint8_t *>
						       &parity )
	    const
	= 0;
	virtual void		encode(
				    const std::pair<uint16_t *, uint16_t *>
						       &data )
	    const
	= 0;
	virtual void		encode(
				    const std::pair<const uint16_t *, const uint16_t *>
						       &data,
				    const std::pair<uint16_t *, uint16_t *>
						       &parity )
	    const
	= 0;
	virtual void		encode(
				    const std::pair<uint32_t *, uint32_t *>
						       &data )
	    const
	= 0;
	virtual void		encode(
				    const std::pair<const uint32_t *, const uint32_t *>
						       &data,
				    const std::pair<uint32_t *, uint32_t *>
						       &parity )
	    const
	= 0;

	int			decode(
				    std::string	       &data,
				    std::vector<int>   *erasure	= 0 )
	    const
	{
	    typedef uint8_t	uT;
	    typedef std::pair<uT *, uT *>
				uTpair;
	    return decode( uTpair( (uT *)&data.front(), (uT *)&data.front() + data.size() ),
			   erasure );
	}

	int			decode(
				    std::string	       &data,
				    std::string	       &parity,
				    std::vector<int>   *erasure	= 0 )
	    const
	{
	    typedef uint8_t	uT;
	    typedef std::pair<uT *, uT *>
				uTpair;
	    return decode( uTpair( (uT *)&data.front(), (uT *)&data.front() + data.size() ),
			   uTpair( (uT *)&parity.front(), (uT *)&parity.front() + parity.size() ),
			   erasure );
	}

	template < typename T >
	int			decode(
				    std::vector<T>     &data,
				    std::vector<int>   *erasure	= 0 )
	    const
	{
	    typedef typename std::make_unsigned<T>::type
				uT;
	    typedef std::pair<uT *, uT *>
				uTpair;
	    return decode( uTpair( (uT *)&data.front(), (uT *)&data.front() + data.size() ),
			   erasure );
	}

	template < typename T >
	int			decode(
				    std::vector<T>     &data,
				    std::vector<T>     &parity,
				    std::vector<int>   *erasure	= 0 )
	    const
	{
	    typedef typename std::make_unsigned<T>::type
				uT;
	    typedef std::pair<uT *, uT *>
				uTpair;
	    return decode( uTpair( (uT *)&data.front(), (uT *)&data.front() + data.size() ),
			   uTpair( (uT *)&parity.front(), (uT *)&parity.front() + parity.size() ),
			   erasure );
	}

	template < typename T, size_t N >
	int			decode(
				    std::array<T,N>    &data,
				    int			pad	= 0, // ignore 'pad' symbols at start of array
				    std::vector<int>   *erasure	= 0 )
	    const
	{
	    typedef typename std::make_unsigned<T>::type
				uT;
	    typedef std::pair<uT *, uT *>
				uTpair;
	    return decode( uTpair( (uT *)&data.front() + pad, (uT *)&data.front() + data.size() ),
			   erasure );
	}

	virtual int		decode(
				    const std::pair<uint8_t *, uint8_t *>
						       &data,
				    std::vector<int>   *erasure	= 0 )
	    const
	= 0;
	virtual int		decode(
				    const std::pair<uint8_t *, uint8_t *>
						       &data,
				    const std::pair<uint8_t *, uint8_t *>
						       &parity,
				    std::vector<int>   *erasure	= 0 )
	    const
	= 0;
	virtual int		decode(
				    const std::pair<uint16_t *, uint16_t *>
						       &data,
				    std::vector<int>   *erasure	= 0 )
	    const
	= 0;
	virtual int		decode(
				    const std::pair<uint16_t *, uint16_t *>
						       &data,
				    const std::pair<uint16_t *, uint16_t *>
						       &parity,
				    std::vector<int>   *erasure	= 0 )
	    const
	= 0;
	virtual int		decode(
				    const std::pair<uint32_t *, uint32_t *>
						       &data,
				    std::vector<int>   *erasure	= 0 )
	    const
	= 0;
	virtual int		decode(
				    const std::pair<uint32_t *, uint32_t *>
						       &data,
				    const std::pair<uint32_t *, uint32_t *>
						       &parity,
				    std::vector<int>   *erasure	= 0 )
	    const
	= 0;
    };
} // namespace ezpwd

// 
// std::ostream << ezpwd::reed_solomon<...>
// 
//     Output a R-S codec description in standard form eg. RS(255,253)
// 
inline
std::ostream		       &operator<<(
				    std::ostream       &lhs,
				    const ezpwd::reed_solomon_base
						       &rhs )
{
    return lhs << "RS(" << rhs.size() << "," << rhs.load() << ")";
}

namespace ezpwd {
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

    //
    // struct reed_solomon - Reed-Solomon codec
    //
    // @TYP, data_t:	A symbol datum; {en,de}code operates on arrays of these
    // @DATUM:		Bits per datum
    // @SYM{BOL}, MM:	Bits per symbol
    // @NN:		Symbols per block (== (1<<MM)-1)
    // @alpha_to:	log lookup table
    // @index_of:	Antilog lookup table
    // @genpoly:	Generator polynomial
    // @NROOTS:		Number of generator roots = number of parity symbols
    // @FCR:		First consecutive root, index form
    // @PRM:		Primitive element, index form
    // @iprim:		prim-th root of 1, index form
    // @PLY:		The primitive generator polynominal functor
    // @MTX, mutex_t:	A std::mutex like object, or a dummy
    // @GRD, guard_t:	A std::lock_guard, or anything that can take a mutex_t
    //
    //     All reed_solomon<T, ...> instances with the same template type parameters share a common
    // (static) set of alpha_to, index_of and genpoly tables.  The first instance to be constructed
    // initializes the tables (optionally protected by a std::mutex/std::lock_guard).
    // 
    //     Each specialized type of reed_solomon implements a specific encode/decode method
    // appropriate to its datum 'TYP' 'data_t'.  When accessed via a generic reed_solomon_base
    // pointer, only access via "safe" (size specifying) containers or iterators is available.
    //
    template < typename TYP, int SYM, int RTS, int FCR, int PRM, class PLY,
	       typename MTX=int, typename GRD=int >
    class reed_solomon
	: public reed_solomon_base {
    public:
	typedef TYP		data_t;
	typedef MTX		mutex_t;
	typedef GRD		guard_t;

	static const size_t	DATUM	= 8 * sizeof data_t();	// bits / data_t
	static const size_t	SYMBOL	= SYM;			// bits / symbol
	static const int	MM	= SYM;
	static const int	NROOTS	= RTS;
	static const int	SIZE	= ( 1 << SYM ) - 1;
	static const int	LOAD	= SIZE - NROOTS;
	static const int	NN	= SIZE;
	static const int	A0	= SIZE;

	virtual size_t		datum() const
	{
	    return DATUM;
	}

	virtual size_t		symbol() const
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
				    const std::pair<uint8_t *, uint8_t *>
						       &data )
	    const
	{
	    encode_mask( data.first, data.second - data.first - NROOTS, data.second - NROOTS );
	}

	virtual void		encode(
				    const std::pair<const uint8_t *, const uint8_t *>
						       &data,
				    const std::pair<uint8_t *, uint8_t *>
						       &parity )
	    const
	{
	    if ( parity.second - parity.first != NROOTS )
		throw std::runtime_error( "reed-solomon: parity length incompatible with number of roots" );
	    encode_mask( data.first, data.second - data.first, parity.first );
	}

	virtual void		encode(
				    const std::pair<uint16_t *, uint16_t *>
						       &data )
	    const
	{
	    encode_mask( data.first, data.second - data.first - NROOTS, data.second - NROOTS );
	}

	virtual void		encode(
				    const std::pair<const uint16_t *, const uint16_t *>
						       &data,
				    const std::pair<uint16_t *, uint16_t *>
						       &parity )
	    const
	{
	    if ( parity.second - parity.first != NROOTS )
		throw std::runtime_error( "reed-solomon: parity length incompatible with number of roots" );
	    encode_mask( data.first, data.second - data.first, parity.first );
	}

	virtual void		encode(
				    const std::pair<uint32_t *, uint32_t *>
						       &data )
	    const
	{
	    encode_mask( data.first, data.second - data.first - NROOTS, data.second - NROOTS );
	}

	virtual void		encode(
				    const std::pair<const uint32_t *, const uint32_t *>
						       &data,
				    const std::pair<uint32_t *, uint32_t *>
						       &parity )
	    const
	{
	    if ( parity.second - parity.first != NROOTS )
		throw std::runtime_error( "reed-solomon: parity length incompatible with number of roots" );
	    encode_mask( data.first, data.second - data.first, parity.first );
	}

	template < typename INP >
	void			encode_mask(
				    const INP	       *data,
				    int			len,
				    INP		       *parity )	// pointer to all NROOTS parity symbols

	    const
	{
	    if ( len < 1 )
		throw std::runtime_error( "reed-solomon: must provide space for all parity and at least one non-parity symbol" );

	    const data_t       	       *dataptr;
	    data_t		       *pariptr;
	    const size_t		INPUT	= 8 * sizeof ( INP );

	    ezpwd::array<data_t,SIZE>	tmp;
	    data_t			msk	= static_cast<data_t>( ~0UL << SYMBOL );
	    static bool			cpy	= DATUM != SYMBOL || DATUM != INPUT;
	    if ( cpy ) {
		// Our DATUM (data_t) size (eg. uint8_t ==> 8, uint16_t ==> 16, uint32_t ==> 32)
		// doesn't exactly match our R-S SYMBOL size (eg. 6), or our INP size; Must copy.
		// The INP data must fit at least the SYMBOL size!
		if ( SYMBOL > INPUT )
		    throw std::runtime_error( "reed-solomon: output data type too small to contain symbols" );
		for ( int i = 0; i < len; ++i )
		    tmp[LOAD - len + i]		= data[i] & ~msk;
		dataptr				= &tmp[LOAD - len];
		pariptr				= &tmp[LOAD];
	    } else {
		// Our R-S SYMBOL size, DATUM size and INP type size exactly matches
		dataptr				= reinterpret_cast<const data_t *>( data );
		pariptr				= reinterpret_cast<data_t *>( parity );
	    }

	    encode( dataptr, len, pariptr );

	    // If we copied/masked data, copy the parity symbols back (may be different sizes)
	    if ( cpy )
		for ( int i = 0; i < NROOTS; ++i )
		    parity[i]			= pariptr[i];
	}
	    
	using reed_solomon_base::decode;
	virtual int		decode(
				    const std::pair<uint8_t *, uint8_t *>
						       &data,
				    std::vector<int>   *erasure	= 0 )
	    const
	{
	    return decode_mask( data.first, data.second - data.first, (uint8_t *)0, erasure );
	}

	virtual int		decode(
				    const std::pair<uint8_t *, uint8_t *>
						       &data,
				    const std::pair<uint8_t *, uint8_t *>
						       &parity,
				    std::vector<int>   *erasure	= 0 )
	    const
	{
	    if ( parity.second - parity.first != NROOTS )
		throw std::runtime_error( "reed-solomon: parity length incompatible with number of roots" );
	    return decode_mask( data.first, data.second - data.first, parity.first, erasure );
	}

	virtual int		decode(
				    const std::pair<uint16_t *, uint16_t *>
						       &data,
				    std::vector<int>   *erasure	= 0 )
	    const
	{
	    return decode_mask( data.first, data.second - data.first, (uint16_t *)0, erasure );
	}

	virtual int		decode(
				    const std::pair<uint16_t *, uint16_t *>
						       &data,
				    const std::pair<uint16_t *, uint16_t *>
						       &parity,
				    std::vector<int>   *erasure	= 0 )
	    const
	{
	    if ( parity.second - parity.first != NROOTS )
		throw std::runtime_error( "reed-solomon: parity length incompatible with number of roots" );
	    return decode_mask( data.first, data.second - data.first, parity.first, erasure );
	}

	virtual int		decode(
				    const std::pair<uint32_t *, uint32_t *>
						       &data,
				    std::vector<int>   *erasure	= 0 )
	    const
	{
	    return decode_mask( data.first, data.second - data.first, (uint32_t *)0, erasure );
	}

	virtual int		decode(
				    const std::pair<uint32_t *, uint32_t *>
						       &data,
				    const std::pair<uint32_t *, uint32_t *>
						       &parity,
				    std::vector<int>   *erasure	= 0 )
	    const
	{
	    if ( parity.second - parity.first != NROOTS )
		throw std::runtime_error( "reed-solomon: parity length incompatible with number of roots" );
	    return decode_mask( data.first, data.second - data.first, parity.first, erasure );
	}

	// 
	// decode_mask	-- mask INP data into valid SYMBOL data
	// 
	///     Incoming data may be in a variety of sizes, and may contain information beyond the
	/// R-S symbol capacity.  For example, we might use a 6-bit R-S symbol to correct the lower
	/// 6 bits of an 8-bit data character.  This would allow us to correct common substitution
	/// errors (such as '2' for '3', 'R' for 'T', 'n' for 'm').
	/// 
	template < typename INP >
	int			decode_mask(
				    INP		       *data,
				    int			len,
				    INP		       *parity	= 0,	// either 0, or pointer to all parity symbols
				    std::vector<int>   *erasure	= 0 )
	    const
	{
	    if ( len < ( parity ? 0 : NROOTS ) + 1 )
		throw std::runtime_error( "reed-solomon: must provide all parity and at least one non-parity symbol" );
	    if ( ! parity ) {
		len			       -= NROOTS;
		parity				= data + len;
	    }

	    data_t	       	       *dataptr;
	    data_t		       *pariptr;
	    const size_t		INPUT	= 8 * sizeof ( INP );

	    ezpwd::array<data_t,SIZE>	tmp;
	    data_t			msk	= static_cast<data_t>( ~0UL << SYMBOL );
	    const bool			cpy	= DATUM != SYMBOL || DATUM != INPUT;
	    if ( cpy ) {
		// Our DATUM (data_t) size (eg. uint8_t ==> 8, uint16_t ==> 16, uint32_t ==> 32)
		// doesn't exactly match our R-S SYMBOL size (eg. 6), or our INP size; Must copy.
		// The INP data must fit at least the SYMBOL size!
		if ( SYMBOL > INPUT )
		    throw std::runtime_error( "reed-solomon: input data type too small to contain symbols" );
		for ( int i = 0; i < len; ++i ) {
		    tmp[LOAD - len + i]		= data[i] & ~msk;
		}
		dataptr				= &tmp[LOAD - len];
		for ( int i = 0; i < NROOTS; ++i ) {
		    if ( data_t( parity[i] ) & msk )
			throw std::runtime_error( "reed-solomon: parity data contains information beyond R-S symbol size" );
		    tmp[LOAD + i]		= parity[i];
		}
		pariptr				= &tmp[LOAD];
	    } else {
		// Our R-S SYMBOL size, DATUM size and INP type size exactly matches
		dataptr				= reinterpret_cast<data_t *>( data );
		pariptr				= reinterpret_cast<data_t *>( parity );
	    }

	    int			corrects;
	    if ( ! erasure ) {
		corrects			= decode( dataptr, len, pariptr );
	    } else {
		// Prepare 'erasure' for the maximum number of corrections, and then reduce
		int		erasures	= erasure->size();
		erasure->resize( NROOTS );
		corrects			= decode( dataptr, len, pariptr,
							  &erasure->front(), erasures );
		erasure->resize( std::max( 0, corrects ));
	    }

	    if ( cpy && corrects > 0 ) {
		for ( int i = 0; i < len; ++i ) {
		    data[i]		       &= msk;
		    data[i]		       |= tmp[LOAD - len + i];
		}
		for ( int i = 0; i < NROOTS; ++i ) {
		    parity[i]			= tmp[LOAD + i];
		}
	    }
	    return corrects;
	}

    protected:
	static mutex_t		mutex;
	static int 		iprim;

#if defined( EZPWD_ARRAY_TEST )
#  warning "EZPWD_ARRAY_TEST: Erroneously declaring alpha_to size!"
	static ezpwd::array<data_t,NN    >
#else
	static ezpwd::array<data_t,NN + 1>
#endif
				alpha_to;
	static ezpwd::array<data_t,NN + 1>
				index_of;
	static ezpwd::array<data_t,NROOTS + 1>
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

	    // Generate Galois field lookup tables
	    index_of[0]			= A0;	// log(zero) = -inf
	    alpha_to[A0]		= 0;	// alpha**-inf = 0
	    PLY			poly;
	    int			sr	= poly( 0 );
	    for ( int i = 0; i < NN; i++ ) {
		index_of[sr]		= i;
		alpha_to[i]		= sr;
		sr			= poly( sr );
	    }
	    // If it's not primitive, exit
	    if ( sr != alpha_to[0] )
		throw std::runtime_error( "reed-solomon: Galois field polynomial not primitive" );

	    // Find prim-th root of 1, used in decoding
	    for ( iprim = 1; (iprim % PRM) != 0; iprim += NN )
		;
	    // prim-th root of 1, index form
	    iprim 		       /= PRM;

	    // Form RS code generator polynomial from its roots
	    genpoly[0]			= 1;
	    for ( int i = 0, root = FCR * PRM; i < NROOTS; i++, root += PRM ) {
		genpoly[i + 1]		= 1;
		// Multiply genpoly[] by  @**(root + x)
		for ( int j = i; j > 0; j-- ) {
		    if ( genpoly[j] != 0 )
			genpoly[j]	= genpoly[j - 1]
			    ^ alpha_to[modnn(index_of[genpoly[j]] + root)];
		    else
			genpoly[j]	= genpoly[j - 1];
		}
		// genpoly[0] can never be zero
		genpoly[0]		= alpha_to[modnn(index_of[genpoly[0]] + root)];
	    }
	    // convert genpoly[] to index form for quicker encoding
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
	    // Check length parameter for validity
	    int			pad	= NN - NROOTS - len;
	    if ( pad < 0 || pad >= NN )
		throw std::runtime_error( "reed-solomon: data length incompatible with block size and error correction symbols" );
	    for ( int i = 0; i < NROOTS; i++ )
		parity[i]		= 0;
	    for ( int i = 0; i < len; i++ ) {
		data_t		feedback= index_of[data[i] ^ invmsk ^ parity[0]];
		if ( feedback != A0 )
		    for ( int j = 1; j < NROOTS; j++ )
			parity[j]       ^= alpha_to[modnn(feedback + genpoly[NROOTS - j])];

		// Shift; was: memmove( &par[0], &par[1], ( sizeof par[0] ) * ( NROOTS - 1 ));
		std::rotate( parity, parity + 1, parity + NROOTS );
		if ( feedback != A0 )
		    parity[NROOTS - 1]	= alpha_to[modnn(feedback + genpoly[0])];
		else
		    parity[NROOTS - 1]	= 0;
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
	    typedef ezpwd::array< data_t, NROOTS >
				data_nroots;
	    typedef ezpwd::array< data_t, NROOTS+1 >
				data_nroots_1;
	    typedef ezpwd::array< int, NROOTS >
				ints_nroots;

	    data_nroots_1	lambda  { { 0 } };
	    data_nroots		syn;
	    data_nroots_1	b;
	    data_nroots_1	t;
	    data_nroots_1	omega;
	    ints_nroots		root;
	    data_nroots_1	reg;
	    ints_nroots		loc;
	    int			count	= 0;

	    // Check length parameter and erasures for validity
	    int			pad	= NN - NROOTS - len;
	    if ( pad < 0 || pad >= NN )
		throw std::runtime_error( "reed-solomon: data length incompatible with block size and error correction symbols" );
	    if ( no_eras ) {
		if ( no_eras > NROOTS )
		    throw std::runtime_error( "reed-solomon: number of erasures exceeds capacity (number of roots)" );
		for ( int i = 0; i < no_eras; ++i )
		    if ( eras_pos[i] < 0 || eras_pos[i] >= len + NROOTS )
			throw std::runtime_error( "reed-solomon: erasure positions outside data+parity" );
	    }

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
		discr_r			= index_of[discr_r];	// Index form
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
#if DEBUG >= 2
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
	    //
	    // Compute err+eras evaluator poly omega(x) = s(x)*lambda(x) (modulo x**NROOTS). in
	    // index form. Also find deg(omega).
	    //
	    deg_omega 			= deg_lambda - 1;
	    for ( int i = 0; i <= deg_omega; i++ ) {
		data_t		tmp	= 0;
		for ( int j = i; j >= 0; j-- ) {
		    if (( syn[i - j] != A0 ) && ( lambda[j] != A0 ))
			tmp	       ^= alpha_to[modnn(syn[i - j] + lambda[j])];
		}
		omega[i]		= index_of[tmp];
	    }

	    //
	    // Compute error values in poly-form. num1 = omega(inv(X(l))), num2 = inv(X(l))**(fcr-1)
	    // and den = lambda_pr(inv(X(l))) all in poly-form
	    //
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
    // Define the static reed_solomon<...> members; allowed in header for template types.
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
#if defined( EZPWD_ARRAY_TEST )
#  warning "EZPWD_ARRAY_TEST: Erroneously defining alpha_to size!"
        ezpwd::array< TYP, reed_solomon< TYP, SYM, RTS, FCR, PRM, PLY, MTX, GRD >::NN     >
#else
        ezpwd::array< TYP, reed_solomon< TYP, SYM, RTS, FCR, PRM, PLY, MTX, GRD >::NN + 1 >
#endif
					reed_solomon< TYP, SYM, RTS, FCR, PRM, PLY, MTX, GRD >::alpha_to;
    template < typename TYP, int SYM, int RTS, int FCR, int PRM, class PLY, typename MTX, typename GRD >
        ezpwd::array< TYP, reed_solomon< TYP, SYM, RTS, FCR, PRM, PLY, MTX, GRD >::NN + 1 >
					reed_solomon< TYP, SYM, RTS, FCR, PRM, PLY, MTX, GRD >::index_of;
    template < typename TYP, int SYM, int RTS, int FCR, int PRM, class PLY, typename MTX, typename GRD >
        ezpwd::array< TYP, reed_solomon< TYP, SYM, RTS, FCR, PRM, PLY, MTX, GRD >::NROOTS + 1 >
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
    // PLY, FCR and PRM values are provided for various SYMBOL sizes, along with appropriate basic
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

    // 
    // ezpwd::base64 -- transform individual characters between 6-bit binary and base64
    // 
    //     The char values [0,64) are mapped by base64::encode onto:
    // 
    //         ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/
    // 
    // and base64::decode performs the inverse.
    // 
    //    Any characters encountered outside [0,64) by encode and outsside the above set
    // by decode raise an exception.
    // 
    namespace base64 {

	template < typename iter>
        void			encode(
				    iter	begin,
				    iter	end )
	{
	    for ( iter i = begin; i != end; ++i ) {
		if ( *i >= char( 0 ) && *i < char( 26 ))
		    *i	       += 'A';
		else if ( *i >= char( 26 ) && *i < char( 52 ))
		    *i	       += 'a' - 26;
		else if ( *i >= char( 52 ) && *i < char( 62 ))
		    *i	       += '0' - 26 - 26;
		else if ( *i == char( 62 ))
		    *i		= '+';
		else if ( *i == char( 63 ))
		    *i		= '/';
		else
		    throw std::runtime_error( "ezpwd::base64::encode: invalid symbol presented" );
	    }
	}

	inline
	std::string		encode(
				    std::string		symbols )
	{
	    encode( symbols.begin(), symbols.end() );
	    return symbols;
	}

	template < typename iter >
	void			decode(
				    iter		begin,
				    iter		end )
	{
	    for ( iter i = begin; i != end; ++i ) {
		if ( *i >= 'A' && *i <= 'Z' )
		    *i	       -= 'A';
		else if ( *i >= 'a' and *i <= 'z' )
		    *i	       -= 'a' - 26;
		else if ( *i >= '0' and *i <= '9' )
		    *i	       -= char( '0' ) - char( 26 ) - char( 26 );
		else if ( *i == '+' )
		    *i		= char( 62 );
		else if ( *i == '/' )
		    *i		= char( 63 );
		else
		    throw std::runtime_error( "ezpwd::base64::decode: invalid symbol presented" );
	    }
	}

	inline
	std::string		decode(
				    std::string		symbols )
	{
	    decode( symbols.begin(), symbols.end() );
	    return symbols;
	}

    } // namespace ezpwd::base64

    // 
    // ezpwd::corrector -- Apply statistical corrections to a string, returning the confidence
    // 
    //     All methods are static; no instance is required, as this is primarily used to create
    // external language APIs.
    // 
    template < size_t N >
    class corrector {
    public:
	// 
	// parity(<string>) -- Returns 'N' base-64 symbols of R-S parity to the supplied password
	// 
	static std::string	parity(
				    const std::string  &password )
	{
	    std::string		parity;
	    rscodec.encode( password, parity );
	    return base64::encode( parity );
	}

	// 
	// encode(<string>) -- append N base-64 parity symbols to password
	// 
	//     The supplied password buffer size must be sufficient to contain N additional symbols, plus
	// the terminating NUL.  Returns the resultant encoded password size (excluding the NUL).
	// 
	static size_t		encode(
				    std::string        &password )
	{
	    password		       += parity( password );
	    return password.size();
	}

	static size_t		encode(
				    char       	       *password,
				    size_t		size )	// maximum available size
	{
	    size_t		len	= ::strlen( password );	// length w/o terminating NUL
	    if ( len + N + 1 > size )
		throw std::runtime_error( "ezpwd::rspwd::encode password buffer has insufficient capacity" );
	    std::string		par	= parity( std::string( password, password + len ));
	    if ( par.size() != N )
		throw std::runtime_error( "ezpwd::rspwd::encode computed parity with incorrect size" );
	    std::copy( par.begin(), par.end(), password + len );
	    len			       += N;
	    password[len]		= 0;
	    return len;
	}

	// 
	// decode(<string>) -- Applies R-S error correction on the encoded string, removing parity
	// 
	//     Up to 'N' Reed-Solomon parity symbols are examined, to determine if the supplied
	// string is a valid R-S codeword and hence very likely to be correct.
	// 
	//     Returns a confidence strength rating, which is the ratio:
	// 
	//         100 - ( errors * 2 + erasures ) * 100 / parity
	// 
	// if an R-S codeword was solved, and 0.0 otherwise.  If a codeword is solved, but the
	// number of errors and erasures corrected indicates that all parity was consumed, we do not
	// use the corrected string, because there is a chance that our R-S polynomial was
	// overwhelmed with errors and actually returned an incorrect codeword.  Therefore,
	// a solving a codeword using all parity results in 100 - N * 100 / N == 0, which matches
	// the strength of the final 
	// 
	//     Supports the following forms of error/erasure:
	// 
	// 0) Full parity.  All data and parity supplied, and an R-S codeword is solved.
	// 
	// 1) Partial parity.  All data and some parity supplied; remainder are deemed erasures.
	// 
	//     If N > 2, then up to N/2-1 trailing parity terms are marked as erasures.  If the R-S
	// codeword is solved and a safe number of errors are found, then we can have reasonable
	// confidence that the string is correct.
	// 
	//   1a) Erase errors.  Permute the combinations of up to N-1 erasures.
	// 
	// o) Raw password.  No parity terms supplied; not an R-S codeword
	// 
	//     If none of the error/erasure forms succeed, the password is returned unmodified.
	// 
	//
	static
	int			strength(
				    int			corrects,
				    const std::vector<int>&erasure,	// original erasures positions
				    const std::vector<int>&position )	// reported correction positions
	{
	    // -'ve indicates R-S failure.	    
	    if ( corrects < 0 )
		return 0;
	    if ( corrects != position.size() )
		throw std::runtime_error( "inconsistent R-S decode results" );
	    // Any erasures that don't turn out to contain errors are not returned as fixed
	    // positions.  However, they have consumed parity resources.
	    int			erased	= erasure.size();
	    for ( auto e : erasure ) {
		if ( std::find( position.begin(), position.end(), e ) == position.end() ) {
		    ++corrects;
		    ++erased;
		}
	    }
	    int			errors	= corrects - erased;
	    return 100 - double( errors * 2 + erased ) * 100 / N;
	}

	typedef std::map<std::string, std::pair<int, int>>
	    			best_avg_base_t;
	class best_avg
	    : public best_avg_base_t
	{
	public:
	    using best_avg_base_t::begin;
	    using best_avg_base_t::end;
	    using best_avg_base_t::insert;
	    using best_avg_base_t::find;
	    using best_avg_base_t::iterator;
	    using best_avg_base_t::const_iterator;
	    using best_avg_base_t::value_type;
	    using best_avg_base_t::mapped_type;
	    // 
	    // add -- add the given pct to the current average for <string> str
	    // 
	    iterator		add(
				    const std::string  &str,
				    int			pct )
	    {
		iterator	i	= find( str );
		if ( i == end() )
		    i 			= insert( i, value_type( str, mapped_type() ));
		i->second.second       *= i->second.first++;
		i->second.second       += pct;
		i->second.second       /= i->second.first;
		return i;
	    }

	    // 
	    // best -- return the unambiguously best value (>, or == but longer), or end()
	    // 
	    const_iterator	best()
		const
	    {
		const_iterator	top	= end();
		bool		uni	= false;
		for ( const_iterator i = begin(); i != end(); ++i ) {
		    if ( top == end()
			 or i->second > top->second
			 or ( i->second == top->second
			      and i->first.size() > top->first.size())) {
			top		= i;
			uni		= true;
		    } else if ( i->second == top->second
				and i->first.size() == top->first.size()) {
			uni		= false;
		    }
		}
		return uni ? top : end();
	    }

	    // 
	    // sort -- return a multimap indexed by avg --> <string>
	    // flip -- invert a (<string>,(<samples>,<average>)) to (<average>,<string>)
	    // output -- output the <string>: <avg>, sorted by average
	    // 
	    static std::pair<const int,const std::string &>
				flip( const value_type &val )
	    {
		return std::pair<const int,const std::string &>( val.second.second, val.first );
	    }
	    typedef std::multimap<const int,const std::string &>
	    			sorted_t;
	    sorted_t		sort()
		const
	    {
		sorted_t	dst;
		std::transform( begin(), end(), std::inserter( dst, dst.begin() ), flip );
		return dst;
	    }
	    std::ostream       &output(
				    std::ostream       &lhs )
		const
	    {
		for ( auto i : sort() )
		    lhs	<< std::setw( 16 ) << i.second
			<< ": " << std::setw( 3 ) << i.first
			<< std::endl;
		return lhs;
	    }
	};
	   
	static
	int			decode(
				    std::string	       &password )
	{
	    int			confidence;
	    best_avg		best;

	    // Full/Partial parity.  Apply some parity erasure if we have some erasure/correction
	    // capability while maintaining at least one excess parity symbol for verification.
	    // This can potentially result in longer password being returned, if the R-S decoder
	    // accidentally solves a codeword.
	    for ( int era = 0; era < (N+1)/2; ++era ) { // how many parity symbols to deem erased
		// For example, if N=3 (or 4) then (N+1)/2 == 2, and we would only try 1 parity
		// erasure.  This would leave 1 parity symbol to replace the 1 erasure, and 1
		// remaining to validate the integrity of the password.
		std::string	fixed	= password;
		fixed.resize( password.size() + era );
		std::vector<int>	erasure;
		for ( int i = fixed.size() - 1; i > fixed.size() - 1 - era; --i )
		    erasure.push_back( i );
		try {
		    base64::decode( fixed.end() - N, fixed.end() - era );
		    std::vector<int>	position= erasure;
		    int corrects	= rscodec.decode( fixed, &position );
		    confidence		= strength( corrects, erasure, position );
		    if ( confidence > 0 ) {
			std::string candidate( fixed, 0, fixed.size() - N );
			best.add( candidate, confidence );
#if defined( DEBUG ) && DEBUG >= 1
			std::cout
			    << " w/ "	 		<< era << " of " << N
			    << " parity erasures "	<< std::setw( 3 ) << confidence
			    << "% confidence: \"" 	<< password
			    << "\" ==> \""		<< candidate
			    << "\": "
			    << std::endl;
			best.output( std::cout );
#endif
		    }
		} catch ( std::exception &exc ) {
#if defined( DEBUG ) && DEBUG >= 2 // should see only when base64::decode fails
		    std::cout << "invalid part parity password: " << exc.what() << std::endl;
#endif
		}
	    }

	    // Partial parity, but below threshold for usable error detection.  For the first 1 to
	    // (N+1)/2 parity symbols (eg. for N == 3, (N+1)/2 == 1 ), we cannot perform meaningful
	    // error or erasure detection.  However, if we see that the terminal symbols match the
	    // R-S symbols we expect from a correct password, we'll ascribe a partial confidence
	    //
	    // password:    sock1t
	    // w/ 3 parity: sock1tkeB
	    // password ----^^^^^^
	    //                    ^^^--- parity
	    for ( int era = (N+1)/2; era < N; ++era ) { // how many parity symbols are not present
		std::string	fixed	= password;
		int		len	= password.size() - ( N - era );
		fixed.resize( len );
		encode( fixed );
		auto		differs	= std::mismatch( fixed.begin(), fixed.end(), password.begin() );
	        int		par_equ	= differs.second - password.begin();
		if ( par_equ < len || par_equ > len + N )
		    throw std::runtime_error( "miscomputed R-S parity matching length" );
		par_equ		       -= len;
		if ( par_equ > 0 ) {
		    std::string	basic( fixed.begin(), fixed.begin() + len );
		    confidence		=  par_equ * 100 / N; // each worth a normal parity symbol
		    best.add( basic, confidence );
#if defined( DEBUG ) && DEBUG >= 1
			std::cout
			    << " w/ "	 		<< era << " of " << N
			    << " parity missing  "	<< std::setw( 3 ) << confidence
			    << "% confidence: \"" 	<< password
			    << "\" ==> \""		<< basic
			    << " (from computed: \""	<< fixed << "\")"
			    << "\": "
			    << std::endl;
			best.output( std::cout );
#endif
		}
		
	    }

	    // Raw password?  No error/erasure attempts succeeded, if no 'best' w/ confidicen > 0.
	    confidence			= 0;
	    typename best_avg::const_iterator
				bi	= best.best();
	    if ( bi != best.end() ) {
		password		= bi->first;
		confidence		= bi->second.second;
	    }
	    return confidence;
	}

	// 
	// decode(<char*>,<size_t>) -- C interface to decode(<string>)
	// 
	static int		decode(
				    char	       *password,
				    size_t		size )	// maximum available size
	{
	    std::string		corrected( password );
	    int			confidence;
	    try {
		confidence			= decode( corrected );
		if ( corrected.size() + 1 > size )
		    throw std::runtime_error( "password buffer has insufficient capacity" );
		std::copy( corrected.begin(), corrected.end(), password );
		password[corrected.size()]	= 0;
	    } catch ( std::exception &exc ) {
		std::cout << "ezpwd::rspwd::decode failed: " << exc.what() << std::endl;
		confidence 			= 0;
	    }
	    return confidence;
	}
	// 
	// rscodec -- A 6-bit RS(63,63-N) Reed-Solomon codec
	// 
	//     Encodes and decodes R-S symbols over the lower 6 bits of the supplied data.  Requires
	// that the last N (parity) symbols of the data are in the range [0,63].  The excess bits on
	// the data symbols are masked and restored during decoding.
	// 
	static RS_63(63-N)	rscodec;
    };

    template < size_t N >
    RS_63(63-N)			corrector<N>::rscodec;
    
} // namespace ezpwd
    
#endif // _EZPWD_RS
