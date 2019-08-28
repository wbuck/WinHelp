#pragma once
#include "expected.hpp"
#include <string_view>

namespace wh::conv
{
	[[nodiscard]]
	expected_ec_t<std::string> to_utf8( std::wstring_view sv ) noexcept;

	[[nodiscard]]
	expected_ec_t<std::wstring> to_utf16( std::string_view utf8str ) noexcept;
}
