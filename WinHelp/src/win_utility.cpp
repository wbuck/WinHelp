#include "pch.h"
#include "win_utility.hpp"
#include "custom_ptrs.hpp"

namespace wh::util
{
	expected_ec_t<unique_event_ptr> create_event( bool is_mre, 
												  bool initial_state, 
												  const wchar_t* name ) noexcept
	{
		auto signal{ unique_event_ptr{
			CreateEventW( nullptr, is_mre, initial_state, name ) } };

		if( !signal )
		{
			return std::error_code{ static_cast<int32_t>(
				GetLastError( ) ), std::system_category( ) };
		}
		return signal;
	}

	expected_ec_t<void> set_privilege( std::wstring_view priv, bool enable ) noexcept
	{
		HANDLE htoken{ nullptr };

		static const auto error = [ ]( DWORD error ) 
		{
			return std::error_code{ static_cast<int32_t>(
				error ), std::system_category( ) };
		};

		if( !OpenProcessToken( GetCurrentProcess( ),
				TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &htoken ) )
		{
			return error( GetLastError( ) );
		}

		unique_any_ptr<HANDLE, &CloseHandle> phtoken{ htoken };

		LUID luid;
		if( !LookupPrivilegeValueW( nullptr, priv.data( ), &luid ) )
			return error( GetLastError( ) );
		
		TOKEN_PRIVILEGES tp{ };
		tp.PrivilegeCount = 1;
		tp.Privileges[ 0 ].Luid = luid;
		tp.Privileges[ 0 ].Attributes =
			enable ? SE_PRIVILEGE_ENABLED : 0;

		if( !AdjustTokenPrivileges( phtoken.get( ),
									false,
									&tp,
									sizeof( TOKEN_PRIVILEGES ),
									nullptr,
									nullptr ) )
		{
			return error( GetLastError( ) );
		}

		return { };
	}

	std::tuple<DWORD, DWORD> split_size( std::size_t size ) noexcept
	{
		DWORD high{ static_cast<DWORD>( ( size >> 32 ) & 0xFFFFFFFFUL ) };
		DWORD low{ static_cast<DWORD>( size & 0xFFFFFFFFUL ) };
		return { high, low };
	}
}