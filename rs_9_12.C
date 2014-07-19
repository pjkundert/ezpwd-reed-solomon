
#include <ezpwd/rs>
#include <ezpwd/hex>

// 
// 10:10+ code w/ Reed-Solomon Error Correction, and improved locality
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
// Therefore, within 9 symbols we define lat and long with about double the
// precision of 4 decimal digits after the decimal point.
// 
// Errors in the first digits matter much more than serious than errors in the
// last.  Errors in the low bits (keyboard nearby character substitutions) are
// much more likely than errors in high bits (keyboard row or column errors).
// However, with only 1-3 symbols to encode parity symbols in, it is difficult
// to supply multiple parity encodings.
// 
// 1 parity symbol: adequate to act as a check character (like 10:10 codes)
// 2 parity symbols: provide 1 symbol of erasure correction (w/ no errors)
// 3 parity symbols: correct 1 error anywhere w/validation, or up to 3 erasures
// 
// Therefore, we'll provide Reed-Solomon RS(31,28) error correction (5 bit
// symbols, indicating 31 symbols in the field, and 3 roots, therefore up to 28
// data symbols in the field) over the 9 lat/lon data symbols.
// 

// decimal point.  For lat, we have 10,000 parts remaining.  For lon, since we
// are missing 1 bit (1 doubling) of precision before the decimal place, we have
// 20,000 parts remaining.
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
// 

//  For Edmonton, we would get 54N, 114W (note:
// rounded, not truncated), which results in 54+90 == 144 lat, 114/2+90 == 57+90
// == 147 lon; 144*180+147 == 26067 == 01100 10111 10011 == "???".
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

	template < typename iter>
        void			encode(
				    iter	begin,
				    iter	end )
	{
	    for ( iter i = begin; i != end; ++i ) {
		if ( *i >= 0 && *i < 32 )
		    *i			= chrs[*i];
		else
		    throw std::runtime_error( "ezpwd::base32::encode: invalid symbol presented" );
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
		if ( ::islower( *i ))
		    *i	        = ::toupper( *i );
		switch ( *i ) {
		case 'Z': *i = '2'; break;
		case 'O': *i = '0'; break;
		case 'S': *i = '5'; break;
		case 'I': *i = '1'; break;
		}
		auto 		chri	= std::search( chrs.begin(), chrs.end(), i, i+1 );
		if ( chri == chrs.end() )
		    throw std::runtime_error( "ezpwd::base32::decode: invalid symbol presented" );
		*i			= chri - chrs.begin();
	    }
	}

	inline
	std::string		decode(
				    std::string		symbols )
	{
	    decode( symbols.begin(), symbols.end() );
	    return symbols;
	}

    } // namespace ezpwd::base32


    class rs_9_12 {
    public:
	double			lat;
	double			lon;

	static const long	lat_parts = 1<<23; // 8,388,608
	static const long	lon_parts = 1<<22; // 4,194,304

	typedef ezpwd::array< std::pair<size_t, size_t>, 9>
				bits_t;
	static const ezpwd::array< std::pair<size_t, size_t>, 9>
				bits;	
	static RS_31(31-3)	rscodec;

				rs_9_12(
				    double	_lat	= 0,
				    double	_lon	= 0 )
				    : lat( _lat )
				    , lon( _lon )
	{
	    ;
	}
	virtual		       ~rs_9_12()
	{
	    ;
	}

	std::string		encode()
	    const
	{
	    long		lat_rem	= ( lat +  90 ) * lat_parts / 180;
	    long		lon_rem	= ( lon + 180 ) * lon_parts / 360;
	    if ( lat_rem < 0 || lat_rem > lat_parts )
		throw std::runtime_error( "ezpwd::rs_9_12::encode: Latitude not in range [-90,90]" );
	    if ( lon_rem < 0 || lon_rem > lon_parts )
		throw std::runtime_error( "ezpwd::rs_9_12::encode: Longitude not in range [-180,180]" );
	    long		lat_mult= lat_parts << 1;
	    long		lon_mult= lon_parts << 1;
	    std::string		res;
	    res.reserve( 9 );
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
		long		lat_val	= lat_rem / lat_mult;
		lat_rem		       -= lat_val * lat_mult;

		lon_mult       	      >>= lon_bits;
		long		lon_val	= lon_rem / lon_mult;
		lon_rem		       -= lon_val * lon_mult;
		char		c	= char( ( lat_val << lon_bits ) | lon_val );
		std::cout << hexify( c ) << "; lat_val: " << lat_val << ", lon_val: " << lon_val << std::endl;
		res		       += c;
	    }

	    // Add the 3 R-S parity symbols and base-32 encode
	    rscodec.encode( res );
	    ezpwd::base32::encode( res.begin(), res.end() );
	    return res;
	}
	std::ostream	       &output( std::ostream &lhs )
	    const
	{
	    std::streamsize	prec	= lhs.precision();
	    lhs.precision( 10 );
	    lhs << std::setprecision( 10 ) << lat << ", " << lon << " == " << encode();
	    lhs.precision( prec );
	    return lhs;
	}
	void			decode( const std::string &s )
	{
	    long		lat_tot	= 0;
	    long		lon_tot	= 0;

	    long		lat_mult= lat_parts << 1;
	    long		lon_mult= lon_parts << 1;
	    std::string		dec;
	    std::copy_if( s.begin(), s.end(), std::back_inserter( dec ),
			  []( char c ) -> bool {
			      return not ::isspace( c );
			  } );
	    base32::decode( dec.begin(), dec.end() );
	    if ( dec.size() > 6 ) {
		// Some R-S parity symbol(s) provided.  See if we can decode/correct.
		std::vector<int> erasure;
		while ( dec.size() < 9 ) {
		    erasure.push_back( dec.size() );
		    dec.resize( dec.size() + 1 );
		}
		int		correct	= rscodec.decode( dec, &erasure );
		std::cout << "Corrected " << correct << " errors/erasures" << std::endl;
		if ( correct < 0 )
		    throw std::runtime_error( "ezpwd::rs_9_12::decode: Error correction failed" );
	    }
	    auto		di	= dec.begin();
	    for ( auto &b : bits ) {
		size_t		lat_bits= b.first;
		size_t		lon_bits= b.second;
		if ( di == dec.end() )
		    break;
		char		c	= *di++;
		long		lat_val	= c >> lon_bits;
		long		lon_val	= c & (( 1 << lon_bits ) - 1 );
		std::cout << hexify( c ) << "; lat_val: " << lat_val << ", lon_val: " << lon_val << std::endl;
		lat_mult	      >>= lat_bits;
		lat_tot		       += lat_val * lat_mult;

		lon_mult	      >>= lon_bits;
		lon_tot		       += lon_val * lon_mult;
	    }
	    lat				= double( lat_tot ) * 180 / lat_parts - 90;
	    lon				= double( lon_tot ) * 360 / lon_parts - 180;
	}
    }; // class rs_9_12

    const rs_9_12::bits_t	rs_9_12::bits = { {
	    rs_9_12::bits_t::value_type( 2, 3 ), // lat, lon bits per symbol
	    rs_9_12::bits_t::value_type( 3, 2 ),
	    rs_9_12::bits_t::value_type( 2, 3 ),

	    rs_9_12::bits_t::value_type( 2, 3 ),
	    rs_9_12::bits_t::value_type( 3, 2 ),
	    rs_9_12::bits_t::value_type( 2, 3 ),

	    rs_9_12::bits_t::value_type( 3, 2 ),
	    rs_9_12::bits_t::value_type( 2, 3 ),
	    rs_9_12::bits_t::value_type( 3, 2 ),
	} };
    RS_31(31-3)			rs_9_12::rscodec;
} // namespace ezpwd

std::ostream		       &operator<<(
				    std::ostream	&lhs,
				    const ezpwd::rs_9_12 &rhs )
{
    return rhs.output( lhs );
}

int				main()
{
    std::string		abc	= "0123abcz";
    std::string		dec	= abc;
    ezpwd::base32::decode( dec.begin(), dec.end() );
    std::cout << ezpwd::hexstr( abc ) << " ==> " << ezpwd::hexstr( dec ) << std::endl;
    std::string		enc	= ezpwd::base32::encode( dec );
    std::cout << ezpwd::hexstr( dec ) << " ==> " << ezpwd::hexstr( enc ) << std::endl;

    double		lat	= 53.555556;
    double		lon	= 113.873889;
    ezpwd::rs_9_12	inp( lat, lon );
    std::cout << inp << std::endl;
    ezpwd::rs_9_12	out;

    // No errors on decode
    std::string		outstr	= inp.encode();
    outstr[3] = '0';
    out.decode( outstr );
    std::cout << out << std::endl;

    // Only 1 parity symbol
    outstr			= inp.encode();
    outstr.resize( outstr.size() - 2 );
    out.decode( outstr );
    std::cout << out << std::endl;

    // Only 1 parity symbol; overwhelm with errors
    outstr			= inp.encode();
    outstr.resize( outstr.size() - 2 );
    outstr[3] = '0';
    try {
	out.decode( outstr );
	std::cout << out << std::endl;
    } catch ( std::exception &exc ) {
	std::cout << "decode failed, as expected, due to error correction failure:" << exc.what() << std::endl;
    }
}
