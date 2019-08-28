#pragma once
#include "custom_ptrs.hpp"
#include "expected.hpp"
#include "flags.hpp"
#include <string>
#include <type_traits>
#include <tuple>
#include <windows.h>
#include <winioctl.h>
#include <system_error>


namespace wh::util
{
	[[nodiscard]]
	expected_ec_t<unique_event_ptr> create_event( bool is_mre, 
												  bool initial_state,
												  const wchar_t* name = nullptr ) noexcept;

	[[nodiscard]]
	expected_ec_t<void> set_privilege( std::wstring_view priv, bool enable ) noexcept;

	template<typename T, 
			 typename U>
	[[nodiscard]]
	inline constexpr auto round_up( T num_to_round, U multiple )
	{
		static_assert( std::is_unsigned_v<T>, "T must be an unsigned" );
		static_assert( std::is_unsigned_v<U>, "U must be an unsigned" );

		return ( ( num_to_round + multiple - 1 ) / multiple ) * multiple;
	}

	template<typename T,
			 typename U>
	[[nodiscard]]
	inline constexpr auto round_down( T num_to_round, U multiple )
	{
		static_assert( std::is_unsigned_v<T>, "T must be an unsigned" );
		static_assert( std::is_unsigned_v<U>, "U must be an unsigned" );

		return ( num_to_round / multiple ) * multiple;
	}

	[[nodiscard]]
	std::tuple<DWORD, DWORD> split_size( std::size_t size ) noexcept;

	template<typename Code>
	[[nodiscard]]
	inline std::error_code get_errorcode( Code ec ) noexcept
	{
		static_assert( std::is_integral_v<Code>, "Type must be an integral" );
		return std::error_code{ static_cast<int32_t>( ec ),
			std::system_category( ) };
	}

	struct system_info : public SYSTEM_INFO
	{ system_info( ) noexcept { GetSystemInfo( this ); } };

	template<wchar_t Drive>
	struct symlink
	{
		static constexpr const wchar_t path[ ]
		{ L'\\', L'\\', L'.', L'\\', Drive, L':', L'\0' };

		constexpr const wchar_t* get_link( ) noexcept { return path; }

		constexpr operator const wchar_t* ( ) noexcept { return get_link( ); };

		constexpr std::size_t size( ) noexcept
		{ return ( sizeof( path ) / sizeof( wchar_t ) ) - 1; }
	};

	template<wchar_t Drive>
	inline expected_ec_t<DWORD> get_drive_sectorsize( ) noexcept
	{
		STORAGE_ACCESS_ALIGNMENT_DESCRIPTOR descriptor{ };
		STORAGE_PROPERTY_QUERY query{ };
		DWORD bytes{ 0 };

		symlink<Drive> link{ };
		auto fexp{ fs::create_file( link.get_link( ),
			access_flag::standard_rights_read,
			share_flag::share_read | share_flag::share_write,
			creation_option::open_existing ) };

		if( !fexp ) return std::move( fexp ).error( );

		auto fhandle{ std::move( fexp ).value( ) };

		query.QueryType = PropertyStandardQuery;
		query.PropertyId = StorageAccessAlignmentProperty;

		if( !DeviceIoControl( fhandle.get( ),
							  IOCTL_STORAGE_QUERY_PROPERTY,
							  &query,
							  sizeof( STORAGE_PROPERTY_QUERY ),
							  &descriptor,
							  sizeof( STORAGE_ACCESS_ALIGNMENT_DESCRIPTOR ),
							  &bytes,
							  nullptr ) )
		{
			return get_errorcode( GetLastError( ) );
		}

		return descriptor.BytesPerPhysicalSector;
	}
}
