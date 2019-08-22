#pragma once
#include "custom_ptrs.hpp"
#include "expected.hpp"
#include <string>
#include <type_traits>
#include <tuple>


namespace wh::util
{
	[[nodiscard]]
	expected_ec_t<unique_event_ptr> create_event( bool is_mre, bool initial_state,
		const wchar_t* name = nullptr ) noexcept;

	[[nodiscard]]
	expected_ec_t<void> set_privilege( std::wstring_view priv, bool enable ) noexcept;

	template<typename T,
	typename = std::enable_if_t<std::is_unsigned_v<T>>>
	[[nodiscard]]
	inline constexpr auto round_up( T num_to_round, T multiple )
	{
		return ( ( num_to_round + multiple - 1 ) / multiple ) * multiple;
	}

	template<typename T,
	typename = std::enable_if_t<std::is_unsigned_v<T>>>
	[[nodiscard]]
	inline constexpr auto round_down( T num_to_round, T multiple )
	{
		return ( num_to_round / multiple ) * multiple;
	}

	[[nodiscard]]
	std::tuple<DWORD, DWORD> split_size( std::size_t size ) noexcept;
}
