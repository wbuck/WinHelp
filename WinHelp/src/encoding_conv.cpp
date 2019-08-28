#include "pch.h"
#include "encoding_conv.hpp"
#include "win_utility.hpp"

namespace wh::conv
{
	using namespace util;
	expected_ec_t<std::string> to_utf8( std::wstring_view utf16str ) noexcept
	{
		int32_t utf16len{ static_cast<int32_t>( utf16str.size( ) ) };

		// Determine the size required for the
		// output buffer.
		int32_t utf8len{ WideCharToMultiByte( 
			CP_UTF8, 0, 
			utf16str.data( ),
			utf16len,
			nullptr, 0, nullptr, nullptr ) };

		if( utf8len == 0 )
			return get_errorcode( GetLastError( ) );		

		std::string utf8str( utf8len, '\0' );

		int32_t bytes_written{ WideCharToMultiByte(
			CP_UTF8, 0,
			utf16str.data( ),
			utf16len,
			utf8str.data( ), 
			utf8len,
			nullptr, nullptr ) };

		if( bytes_written == 0 )
			return get_errorcode( GetLastError( ) );

		return utf8str;
	}

	expected_ec_t<std::wstring> to_utf16( std::string_view utf8str ) noexcept
	{
		int32_t utf8len{ static_cast<int32_t>( utf8str.size( ) ) };

		// Determine the size required for the
		// output buffer.
		int32_t utf16len{ MultiByteToWideChar(
			CP_UTF8, 0,
			utf8str.data( ),
			utf8len,
			nullptr, 0 ) };

		if( utf16len == 0 )
			return get_errorcode( GetLastError( ) );

		std::wstring utf16str( utf16len, L'\0' );

		int32_t bytes_written{ MultiByteToWideChar(
			CP_UTF8, 0,
			utf8str.data( ),
			utf8len,
			utf16str.data( ),
			utf16len ) };

		if( bytes_written == 0 )
			return get_errorcode( GetLastError( ) );

		return utf16str;
	}
}