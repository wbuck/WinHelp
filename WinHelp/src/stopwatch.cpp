#include "pch.h"
#include "stopwatch.hpp"

namespace wh::diag
{
	stopwatch::stopwatch( ) :
		_running{ false }, _counter_frequency{ get_frequency( ) }	{ }

	void stopwatch::start( ) noexcept
	{
		if( !_running )
		{
			_start_ticks = query_counter( );
			_running = true;
		}
	}

	void stopwatch::stop( ) noexcept
	{
		if( _running )
		{
			_elapsed_ticks = query_counter( ) - _start_ticks;
			_running = false;
		}
	}

	void stopwatch::reset( ) noexcept
	{
		_start_ticks = 0;
		_elapsed_ticks = 0;
		_running = false;
	}

	double stopwatch::elapsed_microseconds( ) const noexcept
	{
		return static_cast<double>( ( _elapsed_ticks * 1000000.0 ) / _counter_frequency );
	}

	double stopwatch::elapsed_milliseconds( ) const noexcept
	{
		return static_cast<double>( ( _elapsed_ticks * 1000.0 ) / _counter_frequency );
	}

	double stopwatch::elapsed_seconds( ) const noexcept
	{
		return elapsed_milliseconds( ) / 1000;
	}

	uint64_t stopwatch::elapsed_ticks( ) const noexcept
	{
		return _elapsed_ticks;
	}

	int64_t stopwatch::get_frequency( ) noexcept
	{
		LARGE_INTEGER li;
		QueryPerformanceFrequency( &li );
		return li.QuadPart;
	}

	int64_t stopwatch::query_counter( ) noexcept
	{
		LARGE_INTEGER li;
		QueryPerformanceCounter( &li );
		return li.QuadPart;
	}
}