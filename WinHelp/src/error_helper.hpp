#pragma once
#include "handle_ptr.hpp"
#include "common.hpp"
#include <windows.h>
#include <tchar.h>
#include <optional>
#include <string>



namespace wh::err
{
	using StrType = std::basic_string<TCHAR>;

	[[nodiscard]]
	std::optional<StrType> convt_error( DWORD ec ) NOEXCEPT
	{
		if( ec == ERROR_SUCCESS )
			return std::nullopt;

		LPTSTR buffer{ nullptr };

		DWORD length{ FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr,
			ec,
			MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
			reinterpret_cast<LPTSTR>( &buffer ),
			0, nullptr ) };

		if( length > 0 )
		{
			unique_handle_ptr<TCHAR, & LocalFree> pbuffer{ buffer };
			return StrType{ pbuffer.get( ), length };
		}
		// Grab the code to determine why we were
		// unable to to format the supplied error code.
		auto code{ GetLastError( ) };
		static constexpr int size{ 256 };
		TCHAR msg[ size ]{ };

		length = _stprintf_s( msg, size,
			TEXT( "Failed to retrieve error message string. Error code [%d]" ), code );

#ifdef WHNOTHROW		
		return StrType{ msg, length };
#else
		std::error_code error_code( code, std::system_category( ) );
		throw std::system_error( error_code, "Failed to retrieve formatted error string" );
#endif			
	}
}
