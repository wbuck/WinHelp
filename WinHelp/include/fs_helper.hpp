#pragma once
#include "expected.hpp"
#include "flags.hpp"
#include <string>
#include <shlobj.h>
#include <system_error>
#include "resource.h"

namespace wh::fs
{
	[[nodiscard]]
	expected<std::wstring, std::error_code> get_known_path( const KNOWNFOLDERID& id ) noexcept;

	[[nodiscard]]
	expected<std::wstring, std::error_code> get_temp_path( ) noexcept;

	[[nodiscard]]
	expected<void, std::error_code> create_dir( std::wstring_view dir ) noexcept;

	[[nodiscard]]
	expected<std::wstring, std::error_code> get_current_dir( ) noexcept;

	[[nodiscard]]
	expected<void, std::error_code> remove_dir( std::wstring_view dir ) noexcept;

	[[nodiscard]]
	bool dir_exists( std::wstring_view dir ) noexcept;

	[[nodiscard]]
	expected<wil::unique_hfile, std::error_code> create_file( std::wstring_view filename,
															  access_flag access,
															  share_flag mode,
															  creation_option options,
															  attr_flag flags ) noexcept;
		
}

