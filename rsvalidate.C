
#include <array>
#include <map>
#include <set>
#include <random>
#include <memory>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>

#include <ezpwd/asserter>
#include <ezpwd/rs>
#include <ezpwd/output>

extern "C" {
#include <rs.h> // Phil Karn's implementation
}

static const int		RS_t_LIMIT = 128;

int main()
{
    ezpwd::asserter		assert;
    const int			tests	= 10000;

    // Track the number of success/failures, at varying amounts of error loading. +'ve numbers
    // mean greater parity vs. (erasures + 2 x errors).
    std::map<int,int>		dcodmap;	// Decoder returned success
    std::map<int,int>		succmap;	// Decoder actually succeeded
    std::map<int,int>		failmap;	// Decoder failed to decode to correct codeword

    std::default_random_engine	rnd_gen( (unsigned int)time( 0 ));
    std::uniform_int_distribution<int>
				rnd_dst_bool( 0, 1 );		// random boolean
    std::uniform_int_distribution<int>
				rnd_dst_RS_t( 1, RS_t_LIMIT );	// random parity from 1 to 128
    std::uniform_int_distribution<int>
				rnd_dst_uint8( 0, (1<<8)-1 ); 	// random uint8_t from 0 to 255
    std::uniform_int_distribution<int>
				rnd_dst_uint8_nz( 1, (1<<8)-1 );// random uint8_t from 1 to 255

    typedef std::map<int,std::shared_ptr<ezpwd::reed_solomon_base>>
				rscodec_t;
    rscodec_t			rscodec;	// All available RS codecs
    rscodec[1]				= rscodec_t::mapped_type( new ezpwd::RS<255, 255-1> );
    rscodec[2]				= rscodec_t::mapped_type( new ezpwd::RS<255, 255-2> );
    rscodec[3]				= rscodec_t::mapped_type( new ezpwd::RS<255, 255-3> );
    rscodec[4]				= rscodec_t::mapped_type( new ezpwd::RS<255, 255-4> );
    rscodec[7]				= rscodec_t::mapped_type( new ezpwd::RS<255, 255-7> );
    rscodec[9]				= rscodec_t::mapped_type( new ezpwd::RS<255, 255-9> );
    rscodec[12]				= rscodec_t::mapped_type( new ezpwd::RS<255, 255-12> );
    rscodec[16]				= rscodec_t::mapped_type( new ezpwd::RS<255, 255-16> );
    rscodec[17]				= rscodec_t::mapped_type( new ezpwd::RS<255, 255-17> );
    rscodec[27]				= rscodec_t::mapped_type( new ezpwd::RS<255, 255-27> );
    rscodec[46]				= rscodec_t::mapped_type( new ezpwd::RS<255, 255-46> );
    rscodec[77]				= rscodec_t::mapped_type( new ezpwd::RS<255, 255-77> );
    rscodec[99]				= rscodec_t::mapped_type( new ezpwd::RS<255, 255-99> );
    rscodec[127]			= rscodec_t::mapped_type( new ezpwd::RS<255, 255-127> );
    rscodec[128]			= rscodec_t::mapped_type( new ezpwd::RS<255, 255-128> );
    rscodec[129]			= rscodec_t::mapped_type( new ezpwd::RS<255, 255-129> );
    rscodec[199]			= rscodec_t::mapped_type( new ezpwd::RS<255, 255-199> );

    auto			rsi	= rscodec.end();
    for ( int t = 0; t < tests; ++t ) {

	int			failures= assert.failures;
	std::ostringstream	failmsgs;

	// Traverse through the available R-S codecs
	if ( rsi == rscodec.end() )
	    rsi				= rscodec.begin();
	auto			rs2	= rsi++->second;
	// Select a payload which is a subset of the possible R-S load w/ the given parity
	int 			parity	= rs2->nroots();
	int			payload	= std::uniform_int_distribution<int>( 1, rs2->load() )( rnd_gen );
	int			pad	= rs2->load() - payload;

	// Get a fresh data payload of the maximum possible number of payload data
	std::array<uint8_t,255>	buf;
	for ( auto &c : buf )
	    c	 			= rnd_dst_uint8( rnd_gen );

	failmsgs
	    << "original payload:"
	    << std::endl
	    << std::vector<uint8_t>( buf.begin() + pad, buf.begin() + pad + payload )
	    << std::endl;

	// Phil Karn's standard encoder in enc1, ours in enc2
	std::array<uint8_t,255>	enc1;
	std::copy( buf.begin(), buf.end(), enc1.begin() );
	void	   	       *rs1	= ::init_rs_char( 8, 0x011d, 1, 1, parity, pad );
	::encode_rs_char( rs1, enc1.begin() + pad, enc1.begin() + pad + payload );

	std::array<uint8_t,255>	enc2;
	std::copy( buf.begin(), buf.end(), enc2.begin() );
	rs2->encode( enc2, pad );

	std::vector<uint8_t> cmp2( 255, ' ' );
	int			cmp2cnt	= 0;
	for ( int i = 0; i < 255; ++i ) {
	    if ( enc2[i] != enc1[i] ) {
		cmp2[i]		= '^';
		++cmp2cnt;
	    }
	}
	if ( assert.ISEQUAL( cmp2cnt, 0, "ezpwd::reed_solomon encoder didn't match legacy encoder" ))
	    failmsgs
		<< "legacy encoded:"
		<< std::endl
		<< std::vector<uint8_t>( enc1.begin() + pad, enc1.begin() + pad + payload + parity )
		<< std::endl
		<< *rs2 << " encoded:"
		<< std::endl
		<< std::vector<uint8_t>( enc2.begin() + pad, enc2.begin() + pad + payload + parity )
		<< std::endl
		<< "encoding varies!"
		<< std::endl
		<< std::vector<uint8_t>( cmp2.begin() + pad, cmp2.begin() + pad + payload + parity )
		<< std::endl;

	// 
	// Test max. error and erasure load, to ensure correct decoding (with error detection
	// capacity to spare, after all erasures and errors corrected); Test right past the
	// edge of correction capacity, and complain if it could not correct, when it should
	// be able to!
	// 
	//     erasure             <= parity
	//     2 * error           <= parity
	//     erasure + 2 * error <= parity
	// 
	// The target error load is 100% +/- 10% of the parity capacity.
	std::array<uint8_t,255>	err1;
	std::copy( enc1.begin(), enc1.end(), err1.begin() );
	std::vector<uint8_t>	err1flg( 255, ' ' );

	int			target	= std::uniform_int_distribution<int>( parity * 90 / 100, parity * 110 / 100 )( rnd_gen );
	int			err1cnt	= 0;
	int			era1cnt	= 0;
	switch ( std::uniform_int_distribution<int>( 0, 3 )( rnd_gen )) {
	case 0: default:
	    // No errors.
	    break;

	case 1:
	    // Random number of errors/erasures (sometimes beyond capacity)
	    err1cnt			= std::uniform_int_distribution<int>( 0, target / 2 )( rnd_gen );
	    era1cnt			= target - err1cnt * 2;
	    break;

	case 2:
	    // All errors (max capacity, and sometimes beyond)
	    err1cnt			= target / 2;
	    break;

	case 3:
	    // All erasures (max capacity, and sometimes beyond)
	    era1cnt			= target;
	    break;
	}
	// Make certain we have enough room in the payload and parity for all the errors, erasures.
	// We are going to put each error and erasure at a unique spot.  This will only come up when
	// we use R-S codecs with very large numbers of parity, exceeding the payload load.
	err1cnt				= std::min( err1cnt, payload + parity );
	era1cnt				= std::min( era1cnt, std::max( payload + parity - err1cnt, 0 ));

	// Figure out if we should succeed.  Certainly, if we have excess parity vs. error load.
	// Always, if the error load < parity.  Almost never, if the error load > parity.
	bool			succeed	= ( era1cnt + 2 * err1cnt <= parity );

	if ( ! succeed )
	    failmsgs
		<<  "Decoder overwhemlmed!  Results non-deterministic."
		<< std::endl;

	failmsgs
	    << "Test " << *rs2 << " w/ "
	    << std::setw( 5 ) << payload		<< " payload.  "	
	    << std::setw( 5 ) << t 			<< ": "
	    << std::setw( 3 ) << era1cnt 		<< " erasures + 2 x " 
	    << std::setw( 3 ) << err1cnt 		<< " errors == " 
	    << std::setw( 3 ) << era1cnt + err1cnt * 2	<< " vs. " 
	    << std::setw( 3 ) << parity 		<< " parity"
	    << std::endl;

	for ( int i = 0; i < err1cnt; ++i ) {
	    // Pick a new spot for each error
	    int		err;
	    do {
		err 			= std::uniform_int_distribution<int>( pad, pad + payload + parity - 1 )( rnd_gen );
	    } while ( err1[err] != enc1[err] );
	    err1[err]		       ^= std::uniform_int_distribution<int>( 1, 255 )( rnd_gen );
	    err1flg[err]		= 'e';
	}
	std::vector<int>	era1;
	for ( int i = 0; i < era1cnt; ++i ) {
	    // Pick a new spot for each erasure.  Also ensure the entry is modified.
	    int		era;	
	    do {
		era			= std::uniform_int_distribution<int>( pad, pad + payload + parity - 1 )( rnd_gen );
	    } while ( err1[era] != enc1[era] );
	    era1.push_back( era );
	    err1[era]		       ^= std::uniform_int_distribution<int>( 1, 255 )( rnd_gen );
	    err1flg[era]		= 'x';
	}

	failmsgs
	    << "Erroneous buffer: "
	    << std::endl
	    << std::vector<uint8_t>( err1.begin() + pad, err1.begin() + pad + payload + parity )
	    << std::endl
	    << std::setw( 3 ) << err1cnt << " e (error), "
	    << std::setw( 3 ) << era1cnt << " x (erase)   "
	    << std::endl
	    << std::vector<uint8_t>( err1flg.begin() + pad, err1flg.begin() + pad + payload + parity )
	    << std::endl;

	std::array<uint8_t,255> err2;
	std::copy( err1.begin(), err1.end(), err2.begin() );
	std::vector<int>	era2;
	for ( auto e: era1 )
	    era2.push_back( e - pad );

	// Use the standard decoder, and check the results against the encoded data.  DO NOT attempt
	// to use decoder if our erasure count has already exceeded the parity; the decoder may
	// overrun internal buffers (the 'lambda' buffer, to be precise, when 'no_eras' exceeds
	// 'NROOTS').  Remember; the position of all corrections comes back into the erasures array,
	// so we must expand it to the maximum possible number of corrections -- the parity, or
	// number of roots (NROOTS).  Since we don't know exactly 'til after the call, we'll resize
	// it before and then shrink it after, but only use the first 'era1cnt' entries.
	int			res1	= -1;
	era1.resize( parity );
	if ( era1cnt <= parity ) {
	    res1			= ::decode_rs_char( rs1, &err1.front() + pad, &era1.front(), era1cnt );
	    if ( assert.ISTRUE( res1 <= parity, "Number of corrections incorrectly exceeded parity" ))
		failmsgs
		    << assert
		    << std::endl;
	    if ( res1 > 0 )
		era1.resize( res1 );
	}
		    
	if ( succeed ) {
	    // We expect success!
	    if ( assert.ISEQUAL( res1, era1cnt + err1cnt, "legacy decoder result isn't sum of erasures + errors'" ))
		failmsgs
		    << assert
		    <<  "Decoded buffer:"
		    << std::endl
		    << std::vector<uint8_t>( err1.begin() + pad, err1.begin() + pad + payload + parity )
		    << std::endl;
	} else if ( res1 >= 0 ) {
	    // The decoder may (and usually does, incorrectly, but unavoidably) resolve a
	    // correct "codeword", if the error density is too high...
	    failmsgs
		<< "Decoder return successful completion(" << res1
		<< ", vs " << era1cnt + err1cnt << " errors/erasures), unexpectedly! "
		<< std::endl;
	}

	std::vector<uint8_t>	dif1( 255, ' ' );
	int			dif1cnt	= 0;
	for ( int i = 0; i < 255; ++i ) {
	    if ( err1[i] != enc1[i] ) {
		dif1[i]		= '^';
		++dif1cnt;
	    }
	}
	if ( succeed ) {
	    if ( assert.ISEQUAL( dif1cnt, 0, "legacy decoder failed" ))
		failmsgs
		    << assert
		    << "Differences (original):"
		    << std::endl
		    << std::vector<uint8_t>( dif1.begin() + pad, dif1.begin() + pad + payload + parity )
		    << std::endl;
	} else if ( dif1cnt == 0 && ( err1cnt + era1cnt ) != 0 ) {
	    failmsgs
		<< "Decoding resulted in correct output, unexpectedly!"
		<< std::endl;
	}

	::free_rs_char( rs1 );

	int			res2	= -1;
	std::vector<int>	pos2;
	if ( era1cnt <= parity )
	    res2			= rs2->decode( err2, pad, era2, &pos2 );
	// If error load is below correction threshold, decoder results should always be identical,
	// no matter what.  However, if we've overwhelmed the R-S decoder with errors, the new
	// decoder MAY return different results.  This is because the Phil Karn decoder will return
	// error positions in the "pad" area, if the overwhelmed R-S Galois field polynomial solves
	// to roots located there!  We know this is impossible (the unused "pad" area of the R-S
	// decoder buffer is all zeros).  Therefore, the new decoder detects this situation and
	// returns a failure, instead of the (invalid) erasure positions.
	if ( succeed )
	    if ( assert.ISEQUAL( res2, res1, "ezpwd  decoder return different results" ))
		failmsgs
		    << assert
		    << *rs2 << " decoded buffer:"
		    << std::endl
		    << std::vector<uint8_t>( err2.begin() + pad, err2.begin() + pad + payload + parity )
		    << std::endl;

	if ( res2 >= 0 && assert.ISEQUAL( res2, int( pos2.size() ), "ezpwd  decoder return +'ve value, but different number of positions" ))
	    failmsgs
		<< assert
		<< *rs2 << " decoded buffer:"
		<< std::endl
		<< std::vector<uint8_t>( err2.begin() + pad, err2.begin() + pad + payload + parity )
		<< "; wrong position count: " << pos2.size() << " vs. return value: " << res2
		<< std::endl;
	if ( res1 >= 0 && res2 >= 0 ) {
	    // Both R-S decoders claimed to solve the codeword; they should be equivalent
	    std::vector<uint8_t>	dif2( 255, ' ' );
	    int			dif2cnt	= 0;
	    for ( int i = 0; i < 255; ++i ) {
		if ( err2[i] != enc2[i] ) {
		    dif2[i]		= '^';
		    ++dif2cnt;
		}
	    }
	    if ( assert.ISEQUAL( dif2cnt, dif1cnt ))		// Results should be identical
		failmsgs
		    << assert
		    << "differences:"
		    << std::endl
		    << std::vector<uint8_t>( dif2.begin() + pad, dif2.begin() + pad + payload + parity )
		    << std::endl;
	}

#if ! defined( DEBUG )
	if ( assert.failures != failures )
#endif
	    std::cout
		<< "Detected " << assert.failures - failures << " new failures!"
		<< std::endl
		<< failmsgs.str()
		<< std::endl;


	if ( era1cnt + err1cnt > 0 ) {
	    // Only track tests with some erasures/errors; obviously no errors/erasure tests
	    // will succeed!  Calculate the excess/shortfall in error detection capacity, and
	    // use that to categorize the results.
	    int		capacity	= parity - ( era1cnt + 2 * err1cnt );
	    if ( res1 >= 0 )
		++dcodmap[capacity];	// The decoder claimed to succeed
	    if ( dif1cnt == 0 )
		++succmap[capacity];	// The decoder actually succeeded	
	    else
		++failmap[capacity];	// The decoder did not obtain correct results
	}
    }

    std::set<int>	indices;
    for ( std::map<int,int>::iterator di = dcodmap.begin()
	      ; di != dcodmap.end()
	      ; ++di )
	indices.insert( di->first );
    for ( std::map<int,int>::iterator si = succmap.begin()
	      ; si != succmap.end()
	      ; ++si )
	indices.insert( si->first );
    for ( std::map<int,int>::iterator fi = failmap.begin()
	      ; fi != failmap.end()
	      ; ++fi )
	indices.insert( fi->first );
    std::cout
	<< "parity-(era+2*err)  decoded  successes  failures (-'ve ==> error load > parity capability)"
	<< std::endl;
    for ( std::set<int>::iterator ii = indices.begin()
	      ; ii != indices.end()
	      ; ++ii ) {
	std::cout
	    << std::setw( 18 ) << *ii << "  "
	    << std::setw( 7 ) << dcodmap[*ii] << "  "
	    << std::setw( 9 ) << succmap[*ii] << "  "
	    << std::setw( 8 ) << failmap[*ii]
	    << std::endl;
	if ( *ii >= 0 ) {
	    // Any R-S decode test with a parity >= the error loading should never fail
	    std::cout << assert.ISEQUAL( failmap[*ii], 0 );
	}
    }
    return assert.failures ? 1 : 0;
}
