
/*
 * pid_test	-- Test the <ezpwd/pid> implementation
 */

#include <vector>
#include <map>
#include <set>
#include <iostream>
#include <random>
#include <thread>

#include <ezpwd/asserter>
#include <ezpwd/pid>
#include <ezpwd/units>

#include <curses.h>

std::minstd_rand		randomizer;
std::uniform_int_distribution<uint8_t>
				random_byte( 0, 255 );

template <typename T, typename PRECISION=std::chrono::milliseconds, typename CLOCK=std::chrono::system_clock>
class rocket {

public:
    typedef units::type<T>	units_t;


    //units_t::Mass		mass;				// kg
    typename units_t::Mass	mass;
    typename units_t::Acceleration a0	{ 0 };			// mm/s^2
    typename units_t::Velocity	v0	{ 0 };			// mm/s
    typename units_t::Length	y0;				// mm
    T				goal;				// mm
    typename units_t::Force	thrust;         		// mN (kg.mm/s^2)
    std::optional<T>		thrust_max;         		// mN (kg.mm/s^2)

    T				scale;				// fixed-point scale any physics computations (default 1,000x)

    typedef CLOCK		clock_t;
    typedef PRECISION		precision_t;
    typedef ezpwd::pid<T,PRECISION,CLOCK>
    				autopilot_t;
    autopilot_t			autopilot;

    units_t			constant;			// g, mm, ms

				rocket(
				    T		mass_	= 1,	// kg
				    T		height_	= 0,	// m
				    T		goal_	= 50,	// m
				    T	        thrust_	= 10,	// N
				    std::optional<T>
				    		thrust_max_ = std::nullopt,
				    std::optional<std::chrono::time_point<CLOCK>>
				    		now_	= std::nullopt,
				    std::optional<T>
				    		scale_	= std::nullopt )
				    : mass( mass_ )
				    , y0( height_ )
				    , goal( goal_ )
				    , thrust( thrust_ )
				    , thrust_max( thrust_max_ )
				    , scale( scale_.value_or( T( 1'000 ))) // default scale distances to mm.
				    , constant( 1, scale, 1)
    {
	y0		       *= scale;
	goal		       *= scale;
	thrust		       *= scale;
	if ( thrust_max )
	    *thrust_max	       *= scale;
	autopilot		= autopilot_t( typename autopilot_t::pid_gains_t(  2500, 100, 10000, 0, 1000 ),
					       y0.scalar(), y0.scalar(), thrust.scalar(), 0.0, thrust_max, now_ );
    }


    T				count()
	const
    {
	return autopilot.count();
    }
    
    // 
    // () -- execute a simulation step; returns count of seconds since start
    // 
    //     Computes the effect of the current thrust during the dt from the prior call.
    // Then, updates the thrust (for use by the next invocation) by running the PID loop.
    // 
    T				operator ()(
				    std::optional<std::chrono::time_point<CLOCK>>
				    		now_	= std::nullopt )
    {
        // Compute current altitude 'y', based on elapsed time 'dt' Compute acceleration f = ma,
        // a=f/m, including g, for the last time period's elapsed dt.
	if ( ! now_.has_value() )
	    now_			= CLOCK::now();
	typename units_t::Time	dt( std::chrono::duration_cast<PRECISION>( now_.value() - autopilot.now ).count() );

	if ( dt.scalar() <= 0 )
	    return 0;

	auto			a	= thrust / mass - constant.Gravity;
	auto			dv	= a * dt / PRECISION( 1s ).count(); 

        // Compute ending velocity v = v0 + at (delta-v)
        auto			v	= v0 + dv;
	auto			v_ave   = ( v0 + v ) / 2;

	// Clamp y to launch pad, and eliminate -'ve velocity at pad
        auto			dy 	= v_ave * dt / PRECISION( 1s ).count();
	auto			y	= std::max( y0 + dy, typename units_t::Length( 0 ));
	if ( v < typename units_t::Velocity( 0 ) and y <= typename units_t::Length( 0 ))
            v                   	= typename units_t::Velocity( 0 );

        // and compute actual displacement and hence actual net acceleration for period dt
	//auto			v_ave_act = ( y - y0 ) / dt / PRECISION( 1s ).count();

        // we have an average velocity over the time period; we can deduce ending velocity, and
        //from that, the actual net acceleration experienced over the period by a = ( v - v0 ) / t
        //auto			v_act	= ( v_ave_act - v0 ) * 2;
        //auto			a_act	= ( v_act - v0 ) / dt / PRECISION( 1s ).count();

	a0			= a;
	v0			= v;
	y0			= y;	    

	thrust			= typename units_t::Force( autopilot( goal, y0.scalar(), now_ ));
	
	std::ostringstream	oss;
	oss << "rocket:"
	    << " dt: " << std::setw( 10 ) << std::setprecision( 6 ) << dt << " / " << PRECISION( 1s ).count()
	    << " dv: " << std::setw( 10 ) << std::setprecision( 6 ) << dv
	    << " dy: " << std::setw( 10 ) << std::setprecision( 6 ) << dy
	    ;
	    
	mvprintw( LINES - 5, 0, oss.str().c_str() );

	// Finally, return the current simulation runtime.
	return autopilot.count();
    }

    T				altitude()
	const
    {
	return y0.scalar() / scale;
    }

    std::ostream	       &print(
				    std::ostream       &lhs )
    {
	
	lhs << "rocket altitude: "	<< std::setw( 10 ) << std::setprecision( 6 ) << altitude()
	    << "m, ("			<< std::setw( 10 ) << std::setprecision( 6 ) << y0
	    << ") thrust: "		<< std::setw( 10 ) << std::setprecision( 6 ) << thrust	// 	/ scale
	    << " accel: "		<< std::setw( 10 ) << std::setprecision( 6 ) << a0	//	/ scale
	    << " velocity: "		<< std::setw( 10 ) << std::setprecision( 6 ) << v0	//	/ scale
	    << " "			<< autopilot;
	return lhs;
    };
};

// 
// std::ostream << ezpwd::pid
// 
//     Output a PID controller detail
//
template <typename T, typename CLOCK=std::chrono::system_clock, typename PRECISION=std::chrono::milliseconds>
inline
std::ostream		       &operator<<(
				   std::ostream        &lhs,
				   rocket<T,CLOCK,PRECISION>
				   		       &rhs )
{
    return rhs.print( lhs );
}


void test_pid_steady( ezpwd::asserter &assert )
{
    float		setpoint= 1.0;
    float		process	= 2.0;
    float		output	= 5.0;
    auto		now	= std::chrono::system_clock::now();
    auto		control	= ezpwd::pid<double>( { 2.0, 1.0, 2.0 },
					      setpoint, process, output,
					      std::nullopt, std::nullopt, now );

    assert.ISNEAR( control.K.p, 2.000, .001 );
    assert.ISNEAR( control.K.i, 1.000, .001 );
    assert.ISNEAR( control.K.d, 2.000, .001 );
    assert.ISNEAR( control.P,  -1.000, .001 );
    assert.ISNEAR( control.I,   7.000, .001 );

    assert.ISNEAR( control( 1.0, 2.0, now + 100ms ),   4.9000, .001 ); std::cout << control << std::endl;
    assert.ISNEAR( control.P, -1.000, .001 ); assert.ISNEAR( control.I,  6.900, .001 );
    assert.ISNEAR( control( 1.0, 1.9, now + 200ms ),   7.0100, .001 ); std::cout << control << std::endl;
    assert.ISNEAR( control.P, -0.900, .001 ); assert.ISNEAR( control.I,  6.810, .001 );
    assert.ISNEAR( control( 1.0, 1.8, now + 300ms ),   7.1300, .001 ); std::cout << control << std::endl;
    assert.ISNEAR( control.P, -0.800, .001 ); assert.ISNEAR( control.I,  6.730, .001 );
    assert.ISNEAR( control( 1.0, 1.7, now + 400ms ),   7.2600, .001 ); std::cout << control << std::endl;
    assert.ISNEAR( control.P, -0.700, .001 ); assert.ISNEAR( control.I,  6.660, .001 );
    assert.ISNEAR( control( 1.0, 1.6, now + 500ms ),   7.4000, .001 ); std::cout << control << std::endl;
    assert.ISNEAR( control.P, -0.600, .001 ); assert.ISNEAR( control.I,  6.600, .001 );
    assert.ISNEAR( control( 1.0, 1.4, now + 600ms ),   9.7600, .001 ); std::cout << control << std::endl;
    assert.ISNEAR( control.P, -0.400, .001 ); assert.ISNEAR( control.I,  6.560, .001 );
    assert.ISNEAR( control( 1.0, 1.5, now + 700ms ),   3.5100, .001 ); std::cout << control << std::endl;
    assert.ISNEAR( control.P, -0.500, .001 ); assert.ISNEAR( control.I,  6.510, .001 );
    assert.ISNEAR( control( 1.0, 1.3, now + 800ms ),   9.8800, .001 ); std::cout << control << std::endl;
    assert.ISNEAR( control.P, -0.300, .001 ); assert.ISNEAR( control.I,  6.480, .001 );
    assert.ISNEAR( control( 1.0, 1.1, now + 900ms ),  10.2700, .001 ); std::cout << control << std::endl;
    assert.ISNEAR( control( 1.0, 0.9, now + 950ms ),  14.6750, .001 ); std::cout << control << std::endl;
    assert.ISNEAR( control( 1.0, 1.1, now + 980ms ),  -7.0613, .001 ); std::cout << control << std::endl;
    assert.ISNEAR( control( 1.0, 1.0, now + 1s ),  16.4720, .001 ); std::cout << control << std::endl;
    assert.ISNEAR( control( 1.0, 1.0, now + 2s ),   6.4720, .001 ); std::cout << control << std::endl;
    assert.ISNEAR( control( 1.0, 1.1, now + 3s ),   5.9720, .001 ); std::cout << control << std::endl;
    assert.ISNEAR( control( 1.0, 1.1, now + 4s ),   6.0720, .001 ); std::cout << control << std::endl;
    assert.ISNEAR( control( 1.0, 1.1, now + 5s ),   5.9720, .001 ); std::cout << control << std::endl;
    assert.ISNEAR( control( 1.0, 1.05,now + 6s ),   6.1220, .001 ); std::cout << control << std::endl;
    assert.ISNEAR( control( 1.0, 1.05,now + 7s ),   5.9720, .001 ); std::cout << control << std::endl;
    assert.ISNEAR( control( 1.0, 1.01,now + 8s ),   6.1220, .001 ); std::cout << control << std::endl;
    assert.ISNEAR( control( 1.0, 1.0, now + 9s ),   6.0820, .001 ); std::cout << control << std::endl;
    assert.ISNEAR( control( 1.0, 1.0, now +10s ),   6.0620, .001 ); std::cout << control << std::endl;
    assert.ISNEAR( control( 1.0, 1.0, now +11s ),   6.0620, .001 ); std::cout << control << std::endl;
    assert.ISNEAR( control( 1.0, 1.0, now +12s ),   6.0620, .001 ); std::cout << control << std::endl;
    assert.ISNEAR( control( 1.0, 1.0, now +13s ),   6.0620, .001 ); std::cout << control << std::endl;
    assert.ISNEAR( control( 1.0, 1.0, now +14s ),   6.0620, .001 ); std::cout << control << std::endl;
    
}


/*
 * For testing output of time_point<...>
 *
//
// std::ostream << std::chrono::time_point<...>
//
template <typename CLOCK, typename DURATION>
inline
std::ostream		       &operator<<(
				    std::ostream       &lhs,
				    const std::chrono::time_point<CLOCK, DURATION> &rhs )
{
    std::ios_base::fmtflags	flg	= lhs.flags();
    lhs
	<< std::fixed << std::setw( 20 ) << std::setprecision( 3 )
	<< double( std::chrono::duration_cast<std::chrono::milliseconds>( rhs.time_since_epoch() ).count() ) / std::chrono::milliseconds( 1s ).count();
    lhs.flags( flg );
    return lhs;
}
 *
 */

int main()
{
    ezpwd::asserter		assert;

    ezpwd::pid<float>		p1( { }, 10, 10, 5 );
    std::cout << p1.K << ": " << p1 << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds( 1000 ));
    float			p1_out	= p1( 10 );
    assert.ISNEAR( 10.0f, float( p1_out ), .01f );
    std::cout << p1.K << ": " << p1 << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds( 1000 ));

    test_pid_steady( assert );
    std::this_thread::sleep_for(std::chrono::milliseconds( 1000 ));
    
    WINDOW		       *mainwin	= initscr();

    {
	typedef rocket<float>	rocket_f;
	auto 			now	= rocket_f::clock_t::now();

	auto			r	= rocket_f( 1, 0, 25, 0, 20, now, 1 ); // use 1x scale for real-valued simulation
	std::cout << r << std::endl;
	std::this_thread::sleep_for(std::chrono::milliseconds( 1000 ));
    
	for ( float t = 0; t < 10 * rocket_f::precision_t( 1s ).count(); t = r( now )) {
	    auto		row	= LINES - 3 - r.altitude();
	    auto		col	= COLS / 2;
	    mvaddch( row-2, col, '^' );
	    mvaddch( row-1, col, '|' );
	    mvaddch( row-0, col, ";'`^!*.,"[random_byte( randomizer ) % 8] );
	    std::ostringstream	oss;
	    oss
		//<< "now: " << r.autopilot.now << ": "
		<< r;
	    mvprintw( LINES-1, 0, "%s", oss.str().c_str() );
	    refresh();
	    std::this_thread::sleep_for(std::chrono::milliseconds( 50 ));
	    now				= rocket_f::clock_t::now();
	    clear();
	}
    }

    {
	typedef rocket<long int>	rocket_i;
	auto 			now	= rocket_i::clock_t::now();

	auto			r	= rocket_i( 1, 0, 25, 0, 20, now ); // default 1,000x fixed-point physics math
	std::cout << r << std::endl;
	std::this_thread::sleep_for(std::chrono::milliseconds( 1000 ));
    
	for ( float t = 0; t < 10 * rocket_i::precision_t( 1s ).count(); t = r( now )) {
	    auto		row	= LINES - 3 - r.altitude();
	    auto		col	= COLS / 2;
	    mvaddch( row-2, col, '^' );
	    mvaddch( row-1, col, '|' );
	    mvaddch( row-0, col, ";'`^!*.,"[random_byte( randomizer ) % 8] );
	    std::ostringstream	oss;
	    oss
		// << "now: " << r.autopilot.now << ": "
		<< r;
	    mvprintw( LINES-1, 0, "%s", oss.str().c_str() );
	    refresh();
	    std::this_thread::sleep_for(std::chrono::milliseconds( 50 ));
	    now				= rocket_i::clock_t::now();
	    clear();
	}
    }

    delwin( mainwin );
    endwin();
    refresh();

    if ( assert )
	std::cout << assert << std::endl;
    return assert.failures ? 1 : 0;
}
