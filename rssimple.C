#include <rs>
#include <iostream>
#include <iomanip>
#include <cctype>
#include <cstring>
#include <array>
#include <vector>

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
	lhs << std::setfill( '0' ) << std::hex << std::uppercase << (unsigned int)rhs.c << std::setfill( fill );
    }
    lhs.flags( flg );
    return lhs;
}

template< typename TYP, int SYM, int RTS, int FCR, int PRM, class PLY, typename MTX, typename GRD >
inline
std::ostream		       &operator<<(
				    std::ostream       &lhs,
				    ezpwd::reed_solomon< TYP, SYM, RTS, FCR, PRM, PLY, MTX, GRD >
						       &rhs )
{
    return lhs << "RS(" << rhs.nn << "," << rhs.nn - rhs.nroots << ")";
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
	if ( col == 80 ) {
	    lhs << std::endl;
	    col				= 0;
	}
	lhs << hexify( *i );
	++col;
	if ( *i == '\n' )
	    col				= 80;
    }
    return lhs;
}
				    
template < unsigned char, int N >
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

int main() 
{
    typedef RS_255( 253 )	rs_255_253;
    rs_255_253			rs;
    uint8_t			data[255] = "Hello, world!";
    int				len	= strlen( (char *)data );
    int 			parity	= rs.nroots;
    int				eras_pos[rs.nroots] = {0};
  //int				no_eras	= 0;
    uint8_t			corr[rs.nroots] = {0};

    rs.encode( data, len, data + len );
    std::cout << "Encoded: " << std::vector<uint8_t>( data, data+len+parity ) << std::endl;

    for ( int i = 0; i < len + parity; ++i ) {
	data[i]			^= 1 << i % 8;
	std::cout << "Corrupt: " << std::vector<uint8_t>( data, data+len+parity ) << std::endl;

	int 			count	= rs.decode( data, len, data + len, 0, eras_pos, corr );
	std::vector<uint8_t>	fixes( len+parity, '\0' );
	for ( int i = 0; i < count; ++i )
	    fixes[eras_pos[i]] = corr[i];
	std::cout << "Correct: " << fixes << " (" << count << ")" << std::endl;
	std::cout << "Decoded: " << std::vector<uint8_t>( data, data+len+parity ) << std::endl;
    }


}
