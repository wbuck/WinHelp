#pragma once
#include <chrono>
#include <type_traits>
#include <ostream>

namespace wh::diag
{
	template<typename Duration>
	struct is_duration : std::false_type { };

	template<>
	struct is_duration<std::chrono::nanoseconds> : std::true_type { };

	template<>
	struct is_duration<std::chrono::microseconds> : std::true_type { };

	template<>
	struct is_duration<std::chrono::milliseconds> : std::true_type { };

	template<>
	struct is_duration<std::chrono::seconds> : std::true_type { };

	template<>
	struct is_duration<std::chrono::minutes> : std::true_type { };

	template<>
	struct is_duration<std::chrono::hours> : std::true_type { };

	template<typename Duration>
	constexpr bool is_duration_v = is_duration<Duration>::value;

	class stopwatch final
	{
	public:
		stopwatch( );
		stopwatch( stopwatch const& ) = delete;
		stopwatch& operator=( stopwatch const& ) = delete;
	public:
		void start( ) noexcept;
		void stop( ) noexcept;
		void reset( ) noexcept;

		template<typename Duration>
		Duration elapsed( ) const noexcept;

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

	template<typename Duration>
	inline Duration stopwatch::elapsed( ) const noexcept
	{
		using namespace std::chrono;
		static_assert( is_duration_v<Duration>, "Invalid duration type" );

		nanoseconds ns{ ( _elapsed_ticks * 1000000000 ) / _counter_frequency };
		if constexpr( std::is_same_v<Duration, nanoseconds> )
			return ns;
		else 
			return duration_cast<Duration>( ns );
	}	
}

template<typename Duration,
		 typename = std::enable_if_t<wh::diag::is_duration_v<Duration>>>
inline std::ostream & operator<<( std::ostream & stream, const Duration & dur )
{
	stream << dur.count( );
	return stream;
}
