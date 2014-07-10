
#include <array>
#include <vector>
#include <cctype>

#include <rs>
extern "C" {
#include <rs.h>
}

int
main()
{
    std::array<uint8_t,255>	orig;

    for ( int i = 0; i < orig.size(); ++i )
	orig[i]				= i;

    // ensure that each RS encoder produces the same parity symbols
    {
	void		       *ors	= init_rs_char( 8, 0x11d, 1, 1, 2, 0 );
	std::array<uint8_t,255>	odata( orig );
	encode_rs_char( ors, odata.data(), odata.data() + 253 );
	
	RS_255( 253 )		nrs;
	std::array<uint8_t,255>	ndata( orig );
	nrs.encode( ndata.data(), nrs.LOAD, ndata.data() + nrs.LOAD );
	std::cout
	    << "Phil Karn parity: " << std::vector<uint8_t>( odata.data() + 253, odata.data() + 255 ) 
	    << std::endl;
	std::cout
	    << nrs << " parity: " << std::vector<uint8_t>( ndata.data() + 253, ndata.data() + 255 ) 
	    << std::endl;
	
	odata[0]		       ^= 1;
	int				oeras[2];
	int ocorrs			= decode_rs_char( ors, odata.data(), oeras, 0 );

	ndata[0]		       ^= 1;
	int				neras[nrs.NROOTS];
	int ncorrs			= nrs.decode( ndata.data(), nrs.LOAD, ndata.data() + nrs.LOAD,
						      neras, 0 );
	std::cout 
	    << "Phil Karn corrections: " << ocorrs
	    << std::endl;
	std::cout 
	    << nrs << " corrections: " << ncorrs
	    << std::endl;

    }
}
