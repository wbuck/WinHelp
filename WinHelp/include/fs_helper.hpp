#pragma once
#include "expected.hpp"
#include "flags.hpp"
#include "custom_ptrs.hpp"
#include "win_handle.hpp"
#include <string>
#include <shlobj.h>
#include <system_error>

namespace wh::fs
{
	[[nodiscard]]
	expected_ec_t<std::wstring> get_known_path( const KNOWNFOLDERID& id ) noexcept;

	[[nodiscard]]
	expected_ec_t<std::wstring> get_temp_path( ) noexcept;

	[[nodiscard]]
	expected_ec_t<void> create_dir( std::wstring_view dir ) noexcept;

	[[nodiscard]]
	expected_ec_t<std::wstring> get_current_dir( ) noexcept;

	[[nodiscard]]
	expected_ec_t<void> remove_dir( std::wstring_view dir ) noexcept;

	[[nodiscard]]
	bool dir_exists( std::wstring_view dir ) noexcept;

	[[nodiscard]]
	expected_ec_t<unique_file_ptr> create_file( std::wstring_view filename,
												access_flag access,
												share_flag mode,
												creation_option options,
												attr_flag flags = attr_flag::attr_normal ) noexcept;	
	
	[[nodiscard]]
	expected_ec_t<unique_file_ptr> create_mapping( HANDLE file,
												   protection_flag protect,
												   std::size_t size,
												   const wchar_t* name = nullptr ) noexcept;

	[[nodiscard]]
	expected_ec_t<unique_view_ptr> create_view( HANDLE mmf, 
												std::size_t offset, 
											    std::size_t size ) noexcept;
}

