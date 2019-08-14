#pragma once
#include "expected.hpp"
#include <string_view>

namespace wh::conv
{
	[[nodiscard]]
	expected<std::string, std::error_code> to_utf8( std::wstring_view sv ) noexcept;

	[[nodiscard]]
	expected<std::wstring, std::error_code> to_utf16( std::string_view utf8str ) noexcept;
}
