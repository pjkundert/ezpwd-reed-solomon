
#include <math.h> // M_PI
#include <cmath>

#include <cstdint>
#include <ezpwd/rs>
#include <ezpwd/output>

#if defined( DEBUG )
extern "C" {
#include <rs.h> // Phil Karn's implementation
}
#endif // DEBUG

// 
// 5:10 code w/ Reed-Solomon Error Correction, and improved locality
// 
// - each successive symbol provides greater precision
//   - codes nearby each-other are identical in leading characters
// - sub-10m precision achieved in 9 symbols
//   - more than 4 digits of precision in both lat and lon after the decimal
// - from 1 to 3 symbols of Reed-Solomon parity
//   - 1 parity symbol supplies validation w/ strength similar to check character
//   - 2 parity symbols provides correction of 1 lost symbol (no errors)
//   - 3 parity symbols provides correction of any 1 error, with verification,
//     or recovery of up to any 3 lost symbols (with no other errors)
// 

// 
//     To achieve at least 4 decimal digits of precision after the decimal
// point, we must have defined lat to within 1 part in 1,800,000, and lon to
// within 1 part in 3,600,000.  As each symbol supplies bits, we'll refine the
// computed lat/lon further, reducing the outstanding fraction of "parts" yet to
// be defined.
// 
// 
//             bits      
//    symbols   latitude             longitude
//              bits mul   parts      bits mul   parts
//       1      2    4         4      3    8         8
//       2      3    8        32      2    4        32
//       3      2    4       128      3    8       256
// 
//       4      2    4       512      3    8     2,048
//       5      3    8     4,096      2    4     8,192
//       6      2    4    16,384      3    8    65,536
//
//       7      3    8   131,072      2    4   262,144
//       8      2    4   524,288      3    8 2,097,152
//       9      3    8 4,194,304      2    4 8,388,608
//                    /1,800,000            /3,600,000
// 
//     Therefore, within 9 symbols we define lat and long with about double the
// precision of 10:10 code's 4 decimal digits after the decimal point.  This
// yields an approximate lineal precision of 40,075,000m / 8,388,608 == 5m in
// both dimensions.
// 
//     Errors in the first digits matter much more than serious than errors in the
// last.  Errors in the low bits (keyboard nearby character substitutions) are
// much more likely than errors in high bits (keyboard row or column errors).
// However, with only 1-3 symbols to encode parity symbols in, it is difficult
// to supply multiple parity encodings.  What we would like is:
// 
// 1 parity symbol: adequate to act as a check character (like 10:10 codes)
// 2 parity symbols: provide 1 symbol of erasure correction (w/ no other errors)
// 3 parity symbols: correct 1 error anywhere w/validation, or up to 3 erasures
// 
//     Therefore, we'll provide Reed-Solomon RS(31,28) error correction (5 bit
// symbols, indicating 31 symbols in the field, and 3 roots, therefore up to 28
// data symbols in the field) over the 9 lat/lon data symbols.
// 

// 
//     The first 3-tuple encodes roughly the integer lat (-90,90] w/ resolution
// 1 degree and lon (-180,180] w/ resolution of 2 degrees. This is encoded as 3
// 5-bit values, each encoding 2 or 3 bits each of lat and long.
// 
// 15-bit value with the range (0,180*180] or (0,32400], and encoded as 3 5-bit
// characters.  There is no error coding.
// 

// 
//     The first 3-tuple encodes the integer lat (-90,90] w/ resolution 1 degree
// and lon (-180,180] w/ resolution of 2 degrees. This is encoded a 15-bit value
// with the range (0,180*180] or (0,32400], and encoded as 3 5-bit characters.
// There is no error coding.
// 

//     For Edmonton, we would get 54N, 114W (note: rounded, not truncated),
// which results in 54+90 == 144 lat, 114/2+90 == 57+90 == 147 lon; 144*180+147
// == 26067 == 01100 10111 10011 == "???".
// 
//     To achieve at least 4 decimal digits of precision after the decimal
// point, we must have defined lat and long to within 1 part in 10,000 after the
// decimal point.  For lat, we have 10,000 parts remaining.  For lon, since we
// are missing 1 bit (1 doubling) of precision before the decimal place, we have
// 20,000 parts remaining.
// 
//    The second 3-tuple encodes 1 signed digit (plus 2 bits of sub-digit
// precision, eg. .00, .25, .50, .75) of lat and 2 signed digits (with 1 bit of
// sub-digit precision, eg. .0, .5) of lon precision.  This requires a value in
// the range (0, 80*400], or (0,32000] requiring 15 bits (with 2.3% waste).
// There is no error coding.  For Edmonton, we would now get 53.55N, 113.50W.
// 
//     Thus, after 6 symbols "XXX YYY", you have (-180.0,180.0] plus 2 bits
// (eg. .25) of lat precision, and (-360.0,360.0] plus 1 bit (eg. .5) of lon
// precision, and no error correction.
// 
//    Now, we have 10,000/80 == 1 part in 125 remaining for lat, and 20,000/400
// == 1 part in 50 remaining for lon.  So, we need to supply 7 bits (128) for
// lat and 6 bits (64) for lon, totalling 13 bits.  This achieves slightly
// better than 10m precision in both latitude and longitude, leaving 7 bits for
// Reed-Solomon encoding
// 
//    The final 4-tuple is contains 3 signed digits of lat and 3 signed digits of
// lon, requiring a value (0,2000*2000] or (0,4000000) or 21 bits (with 4.6%
// waste).  The remaining 4 bits


// parity is added, applied to the low 2 bits of all previous 3+2 == 5 symbols.
// This allows us to validate AND CORRECT against common keyboard errors on each
// half of the keyboard (mistyping '2'/'3', 'Q'/'W', 'N'/'M') at a very low cost
// of 4 bits; it won't catch errors involving substitutions of a corresponding
// character from a different row (eg. substituting up 'Q'/'A'/'X' for '1').
// However, with the one remaining bit, we provide 1 R-S parity bit encoding bit
// 4/5 of all previous 5 symbols.  This will detect up to one error in row
// substitution (but will not be able to correct it.)
// 
//    The final 4-tuple contains 3 signed digits of lat and 2.5 signed digits of
// lon, requiring a value (0,1000*2000] or (0,2000000) or 21 bits (with 4.6%
// waste).  The remaining 4 bits
// 


// long as there are no other errors.  For example, if 1 symbol gets rubbed off
// of an address label but the others are legible, we can recover the complete
// lat/lon, correct to 1 digit of precision beyond the decimal point (eg. 53.5N,
// 113.5W == Edmonton)!
// 

//     Finally, the last 4-tuple contains 4 digits of lat/lon precision,
// requiring a value in the range (0,10000*10000] or (0,1_000_000_000)
// 
// 

namespace ezpwd {
    // 
    // ezpwd::base32 -- transform individual characters between 5-bit binary and base32
    // 
    //     The char values [0,32) are mapped by base32::encode onto:
    // 
    //         ABCDEFGHJKLMNPQRTUVWXY0123456789
    // 
    // and base32::decode performs the inverse.  In addition to folding lower-case to upper-case,
    // the following mappings occur on decode:
    // 
    //    O -> 0
    //    Z -> 2
    //    S -> 5
    //    I -> 1
    // 
    //    Any characters encountered outside [0,32) by encode and outsside the above set
    // by decode raise an exception.
    // 
    namespace base32 {

	static const ezpwd::array<int,32> chrs = { { // Must be in sorted order
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
		'G', 'H', 'J', 'K', 'L', 'M', 'N', 'P',
		'Q', 'R', 'T', 'U', 'V', 'W', 'X', 'Y'
	    } };

	// 
	// encode(<string>)	-- encode the supplied sequence of data in the domain (0,32] to base-32 
	// encode(<iter>,<iter>)-- encode the supplied std::string of (0,32] symbols in-place to base-32
	// 
	template < typename iter>
        iter			encode(
				    iter	begin,
				    iter	end )
	{
	    for ( iter i = begin; i != end; ++i ) {
		if ( *i >= 0 && *i < 32 )
		    *i			= chrs[*i];
		else
		    throw std::runtime_error( "ezpwd::base32::encode: invalid symbol presented" );
	    }
	    return end;
	}

	inline
	std::string	       &encode(
				    std::string	       &symbols )
	{
	    encode( symbols.begin(), symbols.end() );
	    return symbols;
	}

	// 
	// decode(<iter>,<iter>)-- decode base-32 symbols in-place, collapsing spaces.
	// decode(<string>)	-- decode base-32 symbols in supplied std::string, collapsing spaces, in-place.
	// 
	//     If erasure vector supplied, marks invalid symbols as erasures; otherwise, throws
	// exception.  Ignores whitespace.  Will return an iterator to just after the last output
	// symbol used in the provided range (eg. to shorten the ), leaving any remaining symbols
	// unchanged.  The <string> version returns the same string reference passed in.
	// 
	// NOTE: will quite likely return an iterator before the supplied 'end', indicating
	// that the output has been truncated (shortened), due to collapsing spaces!
	// 
	template < typename iter >
	iter			decode(
				    iter		begin,
				    iter		end,
				    std::vector<int>   *erasure = 0 )
	{
	    if ( erasure )
		erasure->clear();
	    iter		i, o;
	    for ( i = begin, o = begin; i != end; ++i ) {
		if ( ::isspace( *i ))
		    continue;
		if ( ::islower( *i ))
		    *i	        = ::toupper( *i );
		switch ( *i ) {
		case 'Z': *i = '2'; break;
		case 'O': *i = '0'; break;
		case 'S': *i = '5'; break;
		case 'I': *i = '1'; break;
		}
		auto 		chri	= std::search( chrs.begin(), chrs.end(), i, i+1 );
		if ( chri == chrs.end() ) {
		    // Invalid symbol.  Mark as erasure?  Or throw.
		    if ( erasure ) {
			erasure->push_back( o - begin ); // index of offending // symbol in dest
			chri		= chrs.begin();
		    } else {
			throw std::runtime_error( "ezpwd::base32::decode: invalid symbol presented" );
		    }
		}
		*o++			= chri - chrs.begin();
	    }
	    return o;
	}

	inline
	std::string	       &decode(
				    std::string	       &symbols,
				    std::vector<int>   *erasure = 0 )
	{
	    auto		last	= decode( symbols.begin(), symbols.end(), erasure );
	    if ( last != symbols.end() )
		symbols.resize( last - symbols.begin() ); // eliminated some whitespace
	    return symbols;
	}

    } // namespace ezpwd::base32

    template < size_t P=1, size_t L=9 >			// 1 parity + 9 location symbols
    class ezcod_5 {
    public:
	double			lat;			// [-90,+90] angle, degrees
	double			lon;			// [-180,180]

	double			lat_m;			// resolution, meters
	double			lon_m;

	static const uint32_t	lat_parts = 1 << 22;	// [ -90,90 ] / 4,194,304 parts
	static const uint32_t	lon_parts = 1 << 23;	// [-180,180] / 8,388,608 parts

	typedef ezpwd::array< std::pair<size_t, size_t>, 9>
				bits_t;
	static const bits_t	bits;

	static RS_31( 31-P )	rscodec;

				ezcod_5(
				    double	_lat	= 0,
				    double	_lon	= 0 )
				    : lat( _lat )
				    , lon( _lon )
				    , lat_m( 0 )
				    , lon_m( 0 )
	{
	    if ( P < 1 )
		throw std::runtime_error( "ezpwd::ezcod_5:: At least one parity symbol must be specified" );
	    if ( L < 1 || L > 9 )
		throw std::runtime_error( "ezpwd::ezcod_5:: Only 1-9 location symbol may be specified" );
	}
				ezcod_5(
				    const std::string  &str)
				    : ezcod_5()
	{
	    decode( str );
	}
	virtual		       ~ezcod_5()
	{
	    ;
	}

	std::ostream	       &output( std::ostream &lhs )
	    const
	{
	    std::streamsize	prec	= lhs.precision();
	    std::ios_base::fmtflags
				flg	= lhs.flags();
	    lhs.precision( 10 );
	    lhs << std::showpos << std::fixed << std::setprecision( 10 ) << std::setw( 15 ) << lat
		<< std::noshowpos << std::scientific << std::setprecision( 1 )<< " +/-" << lat_m << "m, "
		<< std::showpos << std::fixed << std::setprecision( 10 ) << std::setw( 15 ) << lon
		<< std::noshowpos << std::scientific << std::setprecision( 1 ) << " +/-" << lon_m << "m"
		<< " == " << encode();
	    lhs.precision( prec );
	    lhs.flags( flg );
	    return lhs;
	}

	std::string		encode()
	    const
	{
	    // Convert lat/lon into a fraction of number of parts assigned to each
	    double		lat_frac= ( lat +  90 ) / 180;
	    if ( lat_frac < 0 || lat_frac > 1 )
		throw std::runtime_error( "ezpwd::ezcod_5::encode: Latitude not in range [-90,90]" );
	    uint32_t		lat_rem	= lat_parts * lat_frac;
	    double		lon_frac= ( lon + 180 ) / 360;
	    if ( lon_frac < 0 || lon_frac > 1 )
		throw std::runtime_error( "ezpwd::ezcod_5::encode: Longitude not in range [-180,180]" );
	    uint32_t		lon_rem	= lon_parts * lon_frac;

	    // Initial loop condition; lat/lon multiplier is left at the base multiplier of the
	    // previous loop.
	    unsigned int	lat_mult= lat_parts;
	    unsigned int	lon_mult= lon_parts;

	    std::string		res;
	    res.reserve( L+P+3 );
	    for ( auto &b : bits ) {
		size_t		lat_bits= b.first;
		size_t		lon_bits= b.second;

		// Each set of bits represents the number of times the current multiplier (after
		// division by the number of bits we're outputting) would go into the remainder.
		// Eg. If _mult was 1024, and _rem is 123 and _bits is 3, we're going to put out
		// the next 3 bits of the value 199.  The last value ended removing all multiples of 
		// 1024.  So, we first get the new multiplier. 1024 >> 3 == 128.  So, we're
		// indicating as a 3-bit value, how many multiples of 128 there are in the value
		// 199.  199 / 128 == 1.
		lat_mult       	      >>= lat_bits;
		uint32_t	lat_val	= lat_rem / lat_mult;
		lat_rem		       -= lat_val * lat_mult;

		lon_mult       	      >>= lon_bits;
		uint32_t	lon_val	= lon_rem / lon_mult;
		lon_rem		       -= lon_val * lon_mult;
		char		c	= char( ( lat_val << lon_bits ) | lon_val );
		res		       += c;
	    }

	    // Add the P R-S parity symbols and base-32 encode
	    rscodec.encode( res );
	    ezpwd::base32::encode( res );
	    res.insert( 3,  1, ' ' );
	    res.insert( 7,  1, ' ' );
	    res.insert( 11, 1, ' ' );
	    
	    return res;
	}

	// 
	// decode(<string>)	-- attempt to decode a lat/lon, returning the confidence percentage
	// 
	//     If data but no parity symbols are supplied, no error checking is performed, and the
	// confidence returned will be 0%.  No erasures within the supplied data are allowed (as
	// there is no capacity to correct them), and an exception will be thrown.
	// 
	//     If parity is supplied, then erasures are allowed.  So long as the total number of
	// erasures is <= the supplied parity symbols, then the decode will proceed (using the
	// parity symbols to fill in the erasures), and the returned confidence will reflect the
	// amount of unused parity capacity.  Each erasure consumes one parity symbol to repair.
	//
	int			decode( const std::string &str )
	{
	    int			confidence = 0;

	    // Decode base-32 into a copy, skip whitespace, and mark invalid symbols as erasures.
	    std::vector<int>	erasure;
	    std::string		dec	= str;
	    base32::decode( dec, &erasure );
	    if ( dec.size() > L || erasure.size() > 0 ) {
		// Some R-S parity symbol(s) were provided (or erasures were marked).  See if we can
		// successfully decode/correct, or (at least) use one parity symbol as a check
		// character.  If we identify more erasures than R-S parity, we must fail; we can't
		// recover the data.  This will of course be the case if we have *any* erasures in
		// the data, and no parity.
		size_t		parity	= 0;
		if ( dec.size() > L )
		    parity		= dec.size() - L;
		while ( dec.size() < L + P ) {
		    erasure.push_back( dec.size() );
		    dec.resize( dec.size() + 1 );
		}
		if ( erasure.size() > parity ) {
		    // We cannot do R-S decoding; not enough parity.  If exactly one parity symbol
		    // was provided, and all erasures were due the missing remaining parity symbols,
		    // we can use the existing parity symbol(s) as a "check character", by simply
		    // re-encoding the supplied non-parity data, and see if the generated parity
		    // symbol(s) match the supplied parity.  This is basically the same as the 10:10
		    // code's check character.
		    if ( parity + erasure.size() == P ) {
			std::string chk( dec.begin(), dec.begin() + L );
			rscodec.encode( chk );
			if ( dec[L] != chk[L] )
			    throw std::runtime_error( "ezpwd::ezcod_5::decode: Error correction failed; check character mismatch" );
			confidence	= 100 - 100 * (P-1) / P; // Check character matched; (P-1)/P of confidence gone
		    } else
			throw std::runtime_error( "ezpwd::ezcod_5::decode: Error correction failed; too many erasures" );
		} else {
		    // We can try R-S decoding; we have (at least) enough parity to try to recover
		    // missing symbol(s).
		    std::vector<int>position;
		    int		corrects= rscodec.decode( dec, erasure, &position );
		    if ( corrects < 0 )
			throw std::runtime_error( "ezpwd::ezcod_5::decode: Error correction failed; R-S decode failed" );
		    // Compute confidence, from spare parity capacity.  Since R-S decode will not
		    // return the position of erasures that turn out (by accident) to be correct,
		    // but they have consumed parity capacity, we re-add them into the correction
		    // position vector.  If the R-S correction reports more corrections than the
		    // parity can possibly have handled correctly, (eg. 2 reported erasures and an
		    // unexpected error), then the decode is almost certainly incorrect; fail.
		    confidence		= ezpwd::strength<P>( corrects, erasure, position );
		    if ( confidence < 0 )
			throw std::runtime_error( "ezpwd::ezcod_5::decode: Error correction failed; R-S decode overwhelmed" );
		}
		if ( dec.size() > L )
		    dec.resize( L ); // Discard any parity symbols
	    }

	    // Unpack the supplied location data; we'll take as much as we are given (up to the
	    // maximum 9 symbols supported, yielding <5m resolution).
	    uint32_t		lat_tot	= 0;
	    uint32_t		lon_tot	= 0;

	    uint32_t		lat_mult= lat_parts;
	    uint32_t		lon_mult= lon_parts;

	    auto		di	= dec.begin();
	    for ( auto &b : bits ) {
		size_t		lat_bits= b.first;
		size_t		lon_bits= b.second;
		if ( di == dec.end() )
		    break;
		char		c	= *di++;
		uint32_t	lat_val	= c >> lon_bits;
		uint32_t	lon_val	= c & (( 1 << lon_bits ) - 1 );

		lat_mult	      >>= lat_bits;
		lat_tot		       += lat_val * lat_mult;

		lon_mult	      >>= lon_bits;
		lon_tot		       += lon_val * lon_mult;
	    }
	    lat				= double( lat_tot ) * 180 / lat_parts - 90;
	    lon				= double( lon_tot ) * 360 / lon_parts - 180;

	    // Compute the resolution (in m.) of the decoded lat/lon.  Resolution of Latitude is
	    // linear.  Longitude resolution changes due to the circumference of the earth reducing
	    // as Latitude moves towards the poles.
	    double		lon_circ= 1 * M_PI * 6371000;
	    lat_m			= lon_circ * lat_mult / lat_parts;
	    double		lat_circ= 2 * M_PI * 6371000 * std::cos( lat * M_PI / 180 );
	    lon_m			= lat_circ * lon_mult / lon_parts;
	    
	    return confidence;
	}
    }; // class ezcod_5

    // 
    // ezcod_5::rscodec -- Reed-Solomon parity codec
    // ezcod_5::bits	   -- distribution of lat/lon precision in each code symbol
    // 
    //     Quickly establishes an extra bit of precision for Longitude, and then evenly distributes
    // future precision between lat/lon.
    // 
    template < size_t P, size_t L >
    RS_31( 31-P )		ezcod_5<P,L>::rscodec;
    template < size_t P, size_t L >
    const typename ezcod_5<P,L>::bits_t	ezcod_5<P,L>::bits = { {
	    //  bits per symbol         lat lon
	    ezcod_5<P,L>::bits_t::value_type( 2,  3 ),
	    ezcod_5<P,L>::bits_t::value_type( 2,  3 ),
	    ezcod_5<P,L>::bits_t::value_type( 3,  2 ),
					      
	    ezcod_5<P,L>::bits_t::value_type( 2,  3 ),
	    ezcod_5<P,L>::bits_t::value_type( 3,  2 ),
	    ezcod_5<P,L>::bits_t::value_type( 2,  3 ),
					      
	    ezcod_5<P,L>::bits_t::value_type( 3,  2 ),
	    ezcod_5<P,L>::bits_t::value_type( 2,  3 ),
	    ezcod_5<P,L>::bits_t::value_type( 3,  2 ),
	    //                          --  --
	    //                          22  23
	} };

} // namespace ezpwd

template < size_t P, size_t L >
std::ostream		       &operator<<(
				    std::ostream	&lhs,
				    const ezpwd::ezcod_5<P,L>
				    			&rhs )
{
    return rhs.output( lhs );
}

template < size_t P, size_t L > void
ezcod_5_exercise( const ezpwd::ezcod_5<P,L> &ezc )
{
    // Does location precision scale linearly with the number of symbols provided?  Are errors
    // detected/corrected successfully?

    std::cout
	<< std::endl << std::endl
	<< "Testing EZLOC location coding w/ " << ezc.rscodec.nroots()
	<< " parity; " << ezc.rscodec
	<< " error correction over " << ezc.rscodec.symbol() << "-bit symbols"
	<< std::endl
	<< ezc
	<< std::endl;

    for ( int test = 0; test < 5; ++test ) {
	std::string	manip	= ezc.encode();
	switch ( test ) {
	case 0: std::cout << std::endl << "no errors:" << std::endl;
	    break;
	case 1: std::cout << std::endl << "one erasure: 1/" << P << " parity consumed" << std::endl;
	    manip[8] = '_';
	    break;
	case 2: std::cout << std::endl << "one error: 2/" << P << " parity consumed" << std::endl;
	    manip[1] = ( manip[1] == '0' ? '1' : '0' );
	    break; 
	case 3: std::cout << std::endl << "one error, one erasure; 3/" << P << " parity consumed" << std::endl;
	    manip[8] = '_';
	    manip[1] = ( manip[1] == '0' ? '1' : '0' );
	    break;
	case 4: std::cout << std::endl << "parity capacity overwhelmed" << std::endl;
	    manip[8] = ( manip[8] == '0' ? '1' : '0' );
	    manip[1] = ( manip[1] == '0' ? '1' : '0' );
	    break;
	}
	for ( size_t i = 0; i <= manip.size(); ++i ) {
	    std::string		trunc( manip.begin(), manip.begin() + i );
	    if ( trunc.back() == ' ' )
		continue;
	    trunc.resize( manip.size(), ' ' );
	    ezpwd::ezcod_5<P,L>	code;
	    try {
		int		conf	= code.decode( trunc );
		std::cout
		    << ezpwd::hexstr( trunc ) << " ==> " << code
		    << " (" << std::setw( 3 ) << conf << "%)" << std::endl;
	    } catch ( std::exception &exc ) {
		std::cout
		    << ezpwd::hexstr( trunc ) << " =x> " << exc.what() << std::endl;
	    }
	}
    }
}



int				main()
{
    std::string		abc	= "0123abcz";
    std::string		dec	= abc;
    ezpwd::base32::decode( dec );
    std::cout << ezpwd::hexstr( abc ) << " ==> " << ezpwd::hexstr( dec ) << std::endl;
    std::string		enc	= dec;
    ezpwd::base32::encode( enc );
    std::cout << ezpwd::hexstr( dec ) << " ==> " << ezpwd::hexstr( enc ) << std::endl;

    double		lat	= 53.555556;
    double		lon	= -113.873889;
    ezpwd::ezcod_5<1>	edm1( lat, lon );
    ezcod_5_exercise( edm1 );
    ezpwd::ezcod_5<2>	edm2( lat, lon );
    ezcod_5_exercise( edm2 );
    ezpwd::ezcod_5<3>	edm3( lat, lon );
    ezcod_5_exercise( edm3 );
    ezpwd::ezcod_5<4>	edm4( lat, lon );
    ezcod_5_exercise( edm4 );
    ezpwd::ezcod_5<5>	edm5( lat, lon );
    ezcod_5_exercise( edm5 );


    // Excercise the R-S codecs beyond their correction capability.  This test used to report -'ve
    // error correction positions.  Now, computing -'ve correctly fails the R-S decode, as it
    // indicates that the supplied data's R-S Galois field polynomial solution inferred errors in
    // data we *know* is correct -- the effective block of zero data in the pad (unused) area of the
    // R-S codeword's capacity!

    // Correct encoding w/2 parity: R 3 U   0 8 M   P V T   G Y
    //                      errors:   v             v
    std::string		err2	= " R 0 U   0 8 M   0 V T   G Y ";
    std::string		fix2	= err2;
    ezpwd::base32::decode( fix2 );
    std::vector<int>	pos2;
    int			cor2	= edm2.rscodec.decode( fix2, std::vector<int>(), &pos2  );
    std::string		enc2	= fix2;
    ezpwd::base32::encode( enc2 );
    std::cout
	<< "2 errors (ezpwd::reed_solomon): " << ezpwd::hexstr( err2 )
	<< " --> " << ezpwd::hexstr( enc2 )
	<< "; detected " << cor2 << " errors"
	<< " @" << pos2
	<< std::endl;

#if defined ( DEBUG )
    // Try Phil Karn's R-S codec over RS(31,29), with 2 parity, a capacity of 29 and payload of 9.
    // May compute error positions in "pad" (unused portion), not in supplied data or parity!
    void	       *rs_31_29	= ::init_rs_char( 5, 0x25, 1, 1, 2, 29-9 );
    std::string		fix_31_29	= err2;
    ezpwd::base32::decode( fix_31_29 );
    std::vector<int>	era_31_29;
    era_31_29.resize( 2 );
    int			cor_31_29	= ::decode_rs_char( rs_31_29, (unsigned char *)&fix_31_29.front(),
							    &era_31_29.front(), 0 );
    std::string		enc_31_29	= fix_31_29;
    ezpwd::base32::encode( enc_31_29 );
    era_31_29.resize( std::max( 0, cor_31_29 ));
    std::cout
	<< "2 errors (Phil Karn R-S coded): " << ezpwd::hexstr( err2 )
	<< " --> " << ezpwd::hexstr( enc_31_29 )
	<< "; detected " << cor_31_29 << " errors"
	<< " @" << era_31_29
	<< std::endl;
#endif // DEBUG
    
}
