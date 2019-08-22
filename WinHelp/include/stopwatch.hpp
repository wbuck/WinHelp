#pragma once
#include <chrono>

namespace wh::diag
{
	class stopwatch
	{
	public:
		stopwatch( );
		stopwatch( stopwatch const& ) = delete;
		stopwatch& operator=( stopwatch const& ) = delete;
	public:
		void start( ) noexcept;
		void stop( ) noexcept;
		void reset( ) noexcept;
		double elapsed_microseconds( ) const noexcept;
		double elapsed_milliseconds( ) const noexcept;
		double elapsed_seconds( ) const noexcept;
		uint64_t elapsed_ticks( ) const noexcept;
	private:
		static int64_t get_frequency( ) noexcept;
		static int64_t query_counter( ) noexcept;
	private:
		bool _running;
		int64_t _counter_frequency;
		int64_t _start_ticks;
		int64_t _elapsed_ticks;
	};
}
