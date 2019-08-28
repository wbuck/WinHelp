//#include "fs_helper.hpp"
//#include "encoding_conv.hpp"
//#include "expected.hpp"
//#include "flags.hpp"
//#include "io_req.hpp"
//#include "win_handle.hpp"
//#include "custom_ptrs.hpp"
//#include "stopwatch.hpp"

#include "win_help.hpp"
#include <variant>
#include <iostream>
#include <fstream>
#include <system_error>
#include <array>
#include <vector>
#include <fstream>
#include <tuple>
#include <type_traits>

#ifndef DEBUG_BREAK
#	define DEBUG_BREAK __debugbreak( )
#endif


//57028608
//28514304
//‭114057216‬
static constexpr DWORD _data_size{ 114057216U };
std::vector<char> _buffer_a( _data_size, 'A' );
std::vector<char> _buffer_b( _data_size, 'B' );
std::vector<char> _buffer_c( _data_size, 'C' );
std::vector<char> _buffer_d( _data_size, 'D' );
std::vector<char> _buffer_e( _data_size, 'E' );

static void test_known_path( )
{
	std::cout << "\nKnown path test\n";
	if( auto expected{ winh::get_known_path( FOLDERID_CommonPrograms ) }; expected )
	{
		std::wstring path{ expected.value( ) };
		std::wcout << L"Known path test success. [" << path << L"]\n";
	}
	else
	{
		std::error_code ec{ expected.error( ) };
		std::cout << "Known path test failed: " << ec.message( ) << '\n';
	}
}

static void test_temp_path( )
{
	std::cout << "\nTemp path test\n";
	if( auto expected{ winh::get_temp_path( ) }; expected )
	{
		std::wstring path{ expected.value( ) };
		std::wcout << L"Temp path test success. [" << path << L"]\n";
	}
	else
	{
		std::error_code ec{ expected.error( ) };
		std::cout << "Temp path test failed: " << ec.message( ) << '\n';
	}
}

static void test_current_path( )
{
	std::cout << "\nCurrent path test\n";
	if( auto expected{ winh::get_current_dir( ) }; expected )
	{
		std::wstring path{ expected.value( ) };
		std::wcout << L"Current path test success. [" << path << L"]\n";
	}
	else
	{
		std::error_code ec{ expected.error( ) };
		std::cout << "Current path test failed: " << ec.message( ) << '\n';
	}
}

static void test_create_dir( std::wstring_view dir )
{
	std::cout << "\nCreate directory test\n";
	if( auto expected{ winh::create_dir( dir ) }; expected )
		std::cout << "Create directory test success\n";	
	else
	{
		std::error_code ec{ expected.error( ) };
		std::cout << "Create directory test failed: " << ec.message( ) << '\n';
	}
}

static void test_dir_exists( std::wstring_view dir )
{
	std::cout << "\nDirectory exists test\n";
	if( winh::dir_exists( dir ) )
		std::cout << "Directory exists test success\n";	
}

static void test_create_file( std::wstring_view filepath )
{
	std::cout << "\nCreate file test\n";
	if( auto expected{ winh::create_file( 
		filepath, 
		winh::access_flag::read | winh::access_flag::write, 
		winh::share_flag::share_none,
		winh::creation_option::create_always, 
		winh::attr_flag::flag_delete_on_close ) }; expected )
	{		
		std::cout << "Create file test success\n";	
	}
	else
	{
		std::error_code ec{ expected.error( ) };
		std::cout << "Create file test failed: " << ec.message( ) << '\n';
	}
}

static void test_remove_dir( std::wstring_view dir )
{
	std::cout << "\nRemove directory test\n";
	if( auto expected{ winh::remove_dir( dir ) }; expected )
		std::cout << "Remove directory test success\n";
	else
	{
		std::error_code ec{ expected.error( ) };
		std::cout << "Remove directory test failed: " << ec.message( ) << '\n';
	}
}

static void test_encoding( )
{
	std::wstring utf16str{ L"'Converted text'" };
	std::string utf8str;

	std::cout << "\nUTF16 to UTF8 test\n";
	if( auto expected_utf8{ winh::to_utf8( utf16str ) }; expected_utf8 )
	{
		utf8str = expected_utf8.value( );
		std::cout << "UTF16 to UTF8 conversion success: [" << utf8str << "]\n";

		std::cout << "\nUTF8 to UTF16 test\n";
		if( auto expected_utf16{ winh::to_utf16( utf8str ) };  expected_utf16 )
		{
			std::wstring utf16{ expected_utf16.value( ) };
			std::wcout << L"UTF8 to UTF16 conversion success: [" << utf16str << L"]\n";
		}
		else
		{
			std::error_code ec{ expected_utf16.error( ) };
			std::cout << "UTF8 to UTF16 test failed: " << ec.message( ) << '\n';
		}
	}
	else
	{
		std::error_code ec{ expected_utf8.error( ) };
		std::cout << "UTF16 to UTF8 test failed: " << ec.message( ) << '\n';
	}
}

static void test_pointer( )
{
	winh::unique_file_ptr null_handle{ nullptr };
	if( !null_handle ) std::cout << "Pointer [NULL] test success\n";
	else std::cout << "Pointer test failed: Invalid handle returned true\n";

	winh::unique_file_ptr invalid_handle{ INVALID_HANDLE_VALUE };
	if( !invalid_handle ) std::cout << "Pointer [INVALID] test success\n";
	else std::cout << "Pointer test failed: Invalid handle returned true\n";
}
/*
template<wchar_t Drive>
struct symlink
{
	static constexpr const wchar_t path[ ]
		{ L'\\', L'\\', L'.', L'\\', Drive, L':', L'\0' };

	constexpr const wchar_t* get_link( ) 
	{ return path; }

	constexpr operator const wchar_t* ( ) { return get_link( ); };

	constexpr std::size_t size( ) 
	{ return ( sizeof( path ) / sizeof( wchar_t ) ) - 1; }
};

template<wchar_t Drive>
winh::expected_ec_t<DWORD> get_sector_size( )
{
	STORAGE_ACCESS_ALIGNMENT_DESCRIPTOR descriptor{ };
	STORAGE_PROPERTY_QUERY query{ };
	DWORD bytes{ 0 };

	symlink<Drive> link{ };
	auto fexp{ winh::create_file( link.get_link( ),
		winh::access_flag::standard_rights_read,
		winh::share_flag::share_read | winh::share_flag::share_write,
		winh::creation_option::open_existing ) };

	if( !fexp )
		return std::move( fexp ).error( );

	auto file{ std::move( fexp ).value( ) };

	query.QueryType = PropertyStandardQuery;
	query.PropertyId = StorageAccessAlignmentProperty;

	if( !DeviceIoControl( file.get( ),
						  IOCTL_STORAGE_QUERY_PROPERTY,
						  &query,
						  sizeof( STORAGE_PROPERTY_QUERY ),
						  &descriptor,
						  sizeof( STORAGE_ACCESS_ALIGNMENT_DESCRIPTOR ),
						  &bytes,
						  nullptr ) )
	{
		return winh::get_errorcode( GetLastError( ) );
	}

	return descriptor.BytesPerPhysicalSector;
}

DWORD DetectSectorSize( WCHAR* devName, PSTORAGE_ACCESS_ALIGNMENT_DESCRIPTOR pAlignmentDescriptor )
{
	DWORD                  Bytes = 0;
	BOOL                   bReturn = FALSE;
	DWORD                  Error = NO_ERROR;
	STORAGE_PROPERTY_QUERY Query;

	ZeroMemory( &Query, sizeof( Query ) );

	HANDLE  hFile = CreateFileW( devName,
		STANDARD_RIGHTS_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL );

	if( hFile == INVALID_HANDLE_VALUE ) {
		wprintf( L"  hFile==INVALID_HANDLE_VALUE. GetLastError() returns %lu.\n", Error = GetLastError( ) );
		return Error;
	}

	Query.QueryType = PropertyStandardQuery;
	Query.PropertyId = StorageAccessAlignmentProperty;

	bReturn = DeviceIoControl( hFile,
		IOCTL_STORAGE_QUERY_PROPERTY,
		&Query,
		sizeof( STORAGE_PROPERTY_QUERY ),
		pAlignmentDescriptor,
		sizeof( STORAGE_ACCESS_ALIGNMENT_DESCRIPTOR ),
		&Bytes,
		NULL );

	if( bReturn == FALSE ) {
		wprintf( L"  bReturn==FALSE. GetLastError() returns %lu.\n", Error = GetLastError( ) );
	}

	CloseHandle( hFile );
	return Error;

}

#define ROUND_UP_SIZE(Value,Pow2) ((SIZE_T) ((((ULONG)(Value)) + (Pow2) - 1) & (~(((LONG)(Pow2)) - 1))))
#define ROUND_UP_PTR(Ptr,Pow2)  ((void *) ((((ULONG_PTR)(Ptr)) + (Pow2) - 1) & (~(((LONG_PTR)(Pow2)) - 1))))
*/
static void test_write_normal( std::wstring_view filepath )
{
	auto privexp{ winh::set_privilege( SE_MANAGE_VOLUME_NAME, true ) };
	auto fexpected{ winh::create_file(
		filepath,
		winh::access_flag::write,
		winh::share_flag::share_none,
		winh::creation_option::create_always ) };

	if( !fexpected )
	{
		auto ec{ fexpected.error( ) };
		std::cout << "Failed to create file: " << ec.message( ) << '\n';
		return;
	}
	auto fhandle{ std::move( fexpected ).value( ) };

	LARGE_INTEGER file_size{ 0 };
	file_size.QuadPart = _data_size * 5;

	if( auto exp{ winh::resize_file( fhandle.get( ), file_size.QuadPart ) }; !exp )
		std::cout << "Failed to resize file: " << exp.error( ).message( ) << '\n';
	if( privexp )
	{
		std::cout << "Setting valid data\n";
		if( !SetFileValidData( fhandle.get( ), file_size.QuadPart ) )
		{
			std::error_code ex{ static_cast< int32_t >(
				GetLastError( ) ), std::system_category( ) };
		}
	}

	winh::stopwatch sw{ };
	winh::stopwatch sw2{ };
	sw.start( );
	int32_t req_n{ 1 };
	while( req_n < 6 )
	{
		std::vector<char>* buffer{ nullptr };
		if( req_n == 1 )
			buffer = &_buffer_a;
		else if( req_n == 2 )
			buffer = &_buffer_b;
		else if( req_n == 3 )
			buffer = &_buffer_c;
		else if( req_n == 4 )
			buffer = &_buffer_d;
		else if( req_n == 5 )
			buffer = &_buffer_e;

		sw2.start( );
		DWORD written{ 0 };
		if( !WriteFile( fhandle.get( ), buffer->data( ), buffer->size(), &written, nullptr ) )
		{
			std::error_code ex{ static_cast< int32_t >( GetLastError( ) ), std::system_category( ) };
			std::cout << "Failed: WriteFile - " << ex.message( ) << '\n';
			return;
		}
		sw2.stop( );
		std::cout << "Inner Complete. Elapsed " << sw2.elapsed<std::chrono::microseconds>( ) << "us\n";
		std::cout << "Inner Complete. Elapsed " << sw2.elapsed<std::chrono::milliseconds>( ) << "ms\n";
		sw2.reset( );
		++req_n;
	}
	sw.stop( );
	std::cout << "Complete. Elapsed " << sw.elapsed<std::chrono::microseconds>( ) << "us\n";
	std::cout << "Complete. Elapsed " << sw.elapsed<std::chrono::milliseconds>( ) << "ms\n";
}

static void test_write_nb_overlapped( std::wstring_view filepath )
{
	auto privexp{ winh::set_privilege( SE_MANAGE_VOLUME_NAME, true ) };
	auto fexpected{ winh::create_file(
		filepath,
		winh::access_flag::write,
		winh::share_flag::share_none,
		winh::creation_option::create_always,
		winh::attr_flag::flag_no_buffering | 
		winh::attr_flag::flag_overlapped ) };
	
	if( !fexpected )
	{
		auto ec{ fexpected.error( ) };
		std::cout << "Failed to create file: " << ec.message( ) << '\n';
		return;
	}
	auto fhandle{ std::move( fexpected ).value( ) };

	std::size_t file_size{ _data_size * 5 };

	auto size{ winh::get_drive_sectorsize<L'C'>( ) };
	if( !size )
	{
		std::cout << size.error( ).message( ) << '\n';
		return;
	}
	DWORD sector_size{ size.value( ) };
	std::cout << "Drive C Sector Size: " << sector_size << '\n';
	file_size = winh::round_up( file_size, sector_size );

	if( file_size % _data_size != 0 )
	{
		std::cout << "Invalid size\n";
		return;
	}

	if( auto exp{ winh::resize_file( fhandle.get( ), file_size ) }; !exp )
		std::cout << "Failed to resize file: " << exp.error( ).message( ) << '\n';

	if( privexp )
	{
		std::cout << "Setting valid data\n";
		if( !SetFileValidData( fhandle.get( ), file_size ) )
		{
			std::error_code ex{ static_cast<int32_t>(
				GetLastError( ) ), std::system_category( ) };
		}
	}
	else
	{
		std::cout << "Failed to set priv: " << privexp.error( ).message( ) << '\n';
	}
	
	using unique_aligned_ptr = winh::unique_any_ptr<LPVOID, &_aligned_free>;
	std::array<unique_aligned_ptr, 5> pointers;

	std::vector<char> data( _data_size, 'A' );
	static constexpr int32_t count{ 5 };
	std::array<OVERLAPPED, count> overlapped{ };
	std::array<HANDLE, count> events{ };

	std::cout << "Creating events and allocating memory\n";
	for( int32_t i{ 0 }; i < count; i++ )
	{
		events[ i ] = CreateEventW( nullptr, TRUE, FALSE, nullptr );
		if( !events[ i ] )
		{
			std::error_code ec{ static_cast<int32_t>(
				GetLastError( ) ), std::system_category( ) };
			std::cout << "Failed to create event: " << ec.message( ) << '\n';
			return;
		}

		auto [ high, low ] = winh::split_size( ( std::size_t )_data_size * i );

		overlapped[ i ].hEvent = events[ i ];
		overlapped[ i ].Offset = low;
		overlapped[ i ].OffsetHigh = high;

		pointers[ i ].reset( _aligned_malloc( _data_size, 512 ) );
		char* ptr{ ( char* )pointers[ i ].get( ) };
		std::copy( data.begin( ), data.end( ), ptr );
	}

	std::cout << "Memory allocated starting test\n";
	// Allocate aligned memory.
	//winh::unique_any_ptr<LPVOID, &_aligned_free> uptr{ _aligned_malloc( _data_size, 512 ) };
	//if( !uptr )
	//{
	//	std::cout << "Failed to allocate aligned memory\n";
	//	return;
	//}
	//std::uintptr_t* ptr{ reinterpret_cast<std::uintptr_t*>( uptr.get( ) ) };
	//
	//if( reinterpret_cast<std::uintptr_t>( uptr.get( ) ) % 512 == 0 )
	//	std::cout << "Memory correctly aligned\n";
	//else 
	//	std::cout << "Failed to align memory\n";
	//
	//const std::uintptr_t* out{ reinterpret_cast< std::uintptr_t*>( _buffer_a.data( ) ) };
	//const std::size_t size{ _buffer_a.size( ) / sizeof( std::uintptr_t ) };
	//
	//std::copy( out, out + size, ptr );

	DWORD written{ 0 };
	winh::stopwatch sw;
	winh::stopwatch sw2;
	sw.start( );

	for( int i = 0; i < count; i++ )
	{
		sw2.start( );
		if( !WriteFile( fhandle.get( ), pointers[ i ].get( ), _data_size, nullptr, &overlapped[ i ] ) )
		{
			if( auto code{ GetLastError( ) }; code != ERROR_IO_PENDING )
			{
				std::error_code ex{ static_cast<int32_t>( code ), std::system_category( ) };
				std::cout << "Failed: WriteFile - " << ex.message( ) << '\n';
				return;
			}			
		}
		else
		{
			std::cout << "Write file returned true\n";
		}
		sw2.stop( );		
		std::cout << "Inner Complete. Elapsed " << sw2.elapsed<std::chrono::microseconds>( ) << "us\n";
		std::cout << "Inner Complete. Elapsed " << sw2.elapsed<std::chrono::milliseconds>( ) << "ms\n";
		sw2.reset( );
	}
	
	WaitForMultipleObjects( 5, events.data( ), TRUE, INFINITE );
	sw.stop( );
	std::cout << "Complete. Elapsed " << sw.elapsed<std::chrono::microseconds>( ) << "us\n";
	std::cout << "Complete. Elapsed " << sw.elapsed<std::chrono::milliseconds>( ) << "ms\n";
	//std::cout << "Bytes written " << written << '\n';
}

static void test_write_overlapped( std::wstring_view filepath )
{
	auto privexpected{ winh::set_privilege( SE_MANAGE_VOLUME_NAME, true ) };
	auto fexpected{ winh::create_file(
		filepath,
		winh::access_flag::write,
		winh::share_flag::share_none,
		winh::creation_option::create_always,
		winh::attr_flag::flag_overlapped ) };

	if( !fexpected )
	{
		auto ec{ fexpected.error( ) };
		std::cout << "Failed to create file: " << ec.message( ) << '\n';
		return;
	}
	auto fhandle{ std::move( fexpected ).value( ) };
	if( !SetFileCompletionNotificationModes( 
		fhandle.get( ), FILE_SKIP_SET_EVENT_ON_HANDLE ) )
	{
		std::error_code ex{ static_cast<int32_t>( GetLastError( ) ), std::system_category( ) };
		std::cout << "Failed: SetFileCompletionNotificationModes - " << ex.message( ) << '\n';
	}
	LARGE_INTEGER file_size{ 0 };
	file_size.QuadPart = _data_size * 5;

	if( auto exp{ winh::resize_file( fhandle.get( ), file_size.QuadPart ) }; !exp )
		std::cout << "Failed to resize file: " << exp.error( ).message( ) << '\n';

	if( privexpected )
	{
		std::cout << "Setting valid data\n";
		if( !SetFileValidData( fhandle.get( ), file_size.QuadPart ) )
		{
			std::error_code ex{ static_cast< int32_t >(
				GetLastError( ) ), std::system_category( ) };
		}
	}

	static constexpr int32_t count{ 5 };
	std::vector<winh::write_io_req<std::vector<char>>> requests;

	std::cout << "Creating requests\n";
	for( int32_t i{ 0 }; i < count; i++ )
		requests.emplace_back( _data_size * i );
	

	winh::stopwatch sw{ };	
	winh::stopwatch sw2{ };
	sw.start( );
	int32_t req_n{ 0 };
	for( auto& req : requests )
	{
		std::vector<char>* buffer{ nullptr };
		if( ++req_n == 1 )
			buffer = &_buffer_a;
		else if( req_n == 2 )
			buffer = &_buffer_b;
		else if( req_n == 3 )
			buffer = &_buffer_c;
		else if( req_n == 4 )
			buffer = &_buffer_d;
		else if( req_n == 5 )
			buffer = &_buffer_e;
		
		sw2.start( );
		if( auto result{ req.write_async(
				fhandle.get( ), std::move( *buffer ) ) }; !result )
		{
			std::cout << result.error( ).message( ) << '\n';
			return;
		}
		sw2.stop( );		

		std::cout << "Inner Complete. Elapsed " << sw2.elapsed<std::chrono::microseconds>( ) << "us\n";
		std::cout << "Inner Complete. Elapsed " << sw2.elapsed<std::chrono::milliseconds>( ) << "ms\n";
		sw2.reset( );
	}

	if( auto ec{ winh::wait_completion_req(
		requests, std::chrono::milliseconds{ INFINITE } ) }; !ec )
	{
		std::cerr << "Error waiting " << ec.error( ).message( ) << '\n';
	}
	sw.stop( );

	if( auto ec{ winh::remove_completed_req( fhandle.get( ), requests ) }; !ec )
		std::cerr << "Error writing " << ec.error( ).message( ) << '\n';

	std::cout << "Complete. Elapsed " << sw.elapsed<std::chrono::microseconds>( ) << "us\n";
	std::cout << "Complete. Elapsed " << sw.elapsed<std::chrono::milliseconds>( ) << "ms\n";
}

static void test_read_overlapped( std::wstring_view filepath )
{
	auto fexpected{ winh::create_file(
		filepath,
		winh::access_flag::write | winh::access_flag::read,
		winh::share_flag::share_none,
		winh::creation_option::open_existing,
		winh::attr_flag::flag_overlapped ) };

	if( !fexpected )
	{
		auto ec{ fexpected.error( ) };
		std::cout << "Failed to create file: " << ec.message( ) << '\n';
		return;
	}
	auto fhandle{ std::move( fexpected ).value( ) };

	static constexpr int32_t count{ 4 };
	std::vector<winh::read_io_req> requests;

	std::cout << "Creating requests\n";
	for( int32_t i{ 0 }; i < count; i++ )
		requests.emplace_back( _data_size * i );

	
	winh::stopwatch sw{ };
	sw.start( );

	std::vector<HANDLE> events;
	std::size_t counter{ 0 };
	std::vector<char> buffer( _data_size * 4, '0' );

	for( auto& req : requests )
	{
		char* buf{ buffer.data( ) + ( counter * _data_size ) };
		if( auto result{ req.read_async( fhandle.get( ), buf, _data_size ) }; !result )
		{
			std::cout << result.error( ).message( ) << '\n';
			return;
		}
		events.push_back( req );
		++counter;
	}

	std::cout << "Waiting\n";
	if( auto ec{ winh::wait_completion_req(
		requests, std::chrono::milliseconds{ INFINITE } ) }; !ec )
	{
		std::cerr << "Error waiting " << ec.error( ).message( ) << '\n';
	}
	if( auto ec{ winh::remove_completed_req( fhandle.get( ), requests ) }; !ec )
		std::cerr << "Error writing " << ec.error( ).message( ) << '\n';

	sw.stop( );

	std::cout << "Complete. Elapsed " << sw.elapsed<std::chrono::microseconds>( ) << "us\n";
	std::cout << "Complete. Elapsed " << sw.elapsed<std::chrono::milliseconds>( ) << "ms\n";

}

static void test_overlapped( std::wstring_view filepath )
{
	auto fexpected{ winh::create_file( 
		filepath, 
		winh::access_flag::write | winh::access_flag::read, 
		winh::share_flag::share_none, 
		winh::creation_option::create_always, 
		winh::attr_flag::flag_overlapped ) };

	if( !fexpected )
	{
		auto ec{ fexpected.error( ) };
		std::cout << "Failed to create file: " << ec.message( ) << '\n';
		return;
	}
	auto fhandle{ std::move( fexpected ).value( ) };

	LARGE_INTEGER file_size{ 0 };
	file_size.QuadPart = _data_size * 4;

	SetFilePointerEx( fhandle.get( ), file_size, nullptr, FILE_BEGIN );
	SetEndOfFile( fhandle.get( ) );

	static constexpr int32_t count{ 4 };
	std::array<OVERLAPPED, count> overlapped{ };
	std::array<HANDLE, count> events{ };
	std::array<char*, count> pointers{ };

	std::cout << "Creating events\n";
	for( int32_t i{ 0 }; i < count; i++ )
	{
		events[ i ] = CreateEventW( nullptr, TRUE, FALSE, nullptr );
		if( !events[ i ] )
		{
			std::error_code ec{ static_cast<int32_t>( 
				GetLastError( ) ), std::system_category( ) };
			std::cout << "Failed to create event: " << ec.message( ) << '\n';
			return;
		}
		overlapped[ i ].hEvent = events[ i ];
	}

	LARGE_INTEGER li{ 0 };
	winh::stopwatch sw{ };
	sw.start( );

	for( int32_t i{ 0 }; i < count; i++ )
	{
		switch( i )
		{
		case 0: 
			pointers[ i ] = _buffer_a.data( );
			break;
		case 1:
			pointers[ i ] = _buffer_b.data( );
			break;
		case 2:
			pointers[ i ] = _buffer_c.data( );
			break;
		case 3:
			pointers[ i ] = _buffer_d.data( );
			break;
		}
		overlapped[ i ].Offset = li.LowPart;
		overlapped[ i ].OffsetHigh = li.HighPart;

		if( !WriteFile( fhandle.get( ), pointers[ i ], _data_size, nullptr, &overlapped[ i ] ) )
		{
			if( auto code{ GetLastError( ) }; code != ERROR_IO_PENDING )
			{
				std::error_code ec{ static_cast<int32_t>(
					GetLastError( ) ), std::system_category( ) };
				std::cout << "Failed to write to file: " << ec.message( ) << '\n';
				return;
			}
		}
		li.QuadPart += _data_size;
	}
	std::cout << "Waiting\n";
	WaitForMultipleObjects( 4, events.data( ), TRUE, 60000 );
	sw.stop( );

	std::cout << "Complete. Elapsed " << sw.elapsed<std::chrono::microseconds>( ) << "us\n";
	std::cout << "Complete. Elapsed " << sw.elapsed<std::chrono::milliseconds>( ) << "ms\n";
}


static void test_mmf( std::wstring_view filepath )
{
	auto privexpected{ winh::set_privilege( SE_MANAGE_VOLUME_NAME, true ) };
	auto fexpected{ winh::create_file(
		filepath,
		winh::access_flag::write | winh::access_flag::read,
		winh::share_flag::share_read | winh::share_flag::share_write,
		winh::creation_option::create_always,
		winh::attr_flag::attr_normal ) };

	if( !fexpected )
	{
		auto ec{ fexpected.error( ) };
		std::cout << "Failed to create file: " << ec.message( ) << '\n';
		return;
	}
	auto fhandle{ std::move( fexpected.value( ) ) };
	if( !SetFileCompletionNotificationModes(
		fhandle.get( ), FILE_SKIP_SET_EVENT_ON_HANDLE ) )
	{
		std::error_code ex{ static_cast< int32_t >( GetLastError( ) ), std::system_category( ) };
		std::cout << "Failed: SetFileCompletionNotificationModes - " << ex.message( ) << '\n';
	}

	static const winh::system_info info{ };
	static const std::size_t granularity{ info.dwAllocationGranularity };

	const int64_t view_size{ winh::round_up( _data_size, info.dwAllocationGranularity ) };
	const LARGE_INTEGER file_size{ view_size * 5 };

	if( auto exp{ winh::resize_file( fhandle.get( ), file_size.QuadPart ) }; !exp )
		std::cout << "Failed to resize file: " << exp.error( ).message( ) << '\n';

	if( privexpected )
	{
		std::cout << "Setting valid data\n";
		if( !SetFileValidData( fhandle.get( ), file_size.QuadPart ) )
		{
			std::error_code ex{ static_cast< int32_t >(
				GetLastError( ) ), std::system_category( ) };
		}
	}
	
	if( file_size.QuadPart % view_size != 0 )
	{
		std::cerr << "Invalid view size\n";
		return;
	}

	auto mmfexpected{ winh::create_mapping( fhandle.get( ),
		winh::protection_flag::read_write, 0 ) };

	if( !mmfexpected )
	{
		std::cout << "Failed to map file: " << mmfexpected.error( ).message( ) << '\n';
		return;
	}
	winh::unique_file_ptr mmf{ std::move( mmfexpected ).value( ) };
	if( !mmf )
	{
		std::error_code ec{ static_cast<int32_t>(
			GetLastError( ) ), std::system_category( ) };
		std::cout << "Failed to create MMF: " << ec.message( ) << '\n';
		return;
	}

	static constexpr int32_t count{ 5 };
	std::array<char*, count> pointers{ };

	winh::stopwatch sw{ };
	winh::stopwatch sw2{ };
	sw.start( );
	std::size_t file_offset{ 0 };

	for( int32_t i{ 0 }; i < count; i++ )
	{
		switch( i )
		{
		case 0:
			pointers[ i ] = _buffer_a.data( );
			break;
		case 1:
			pointers[ i ] = _buffer_b.data( );
			break;
		case 2:
			pointers[ i ] = _buffer_c.data( );
			break;
		case 3:
			pointers[ i ] = _buffer_d.data( );
			break;
		case 4:
			pointers[ i ] = _buffer_e.data( );
			break;
		}

		auto viewexpected{ winh::create_view( mmf.get( ), file_offset, _data_size ) };
		if( !viewexpected )
		{
			std::cout << "Failed to create MMF view: " << viewexpected.error( ).message( ) << '\n';
			return;
		}
		winh::unique_view_ptr view{ std::move( viewexpected ).value( ) };
		char* outbuf{ static_cast<char*>( view.get( ) ) };	
		sw2.start( );
		std::copy( pointers[ i ], pointers[ i ] + _data_size, outbuf );
		sw2.stop( );
		std::cout << "Inner Complete. Elapsed " << sw2.elapsed<std::chrono::microseconds>( ) << "us\n";
		std::cout << "Inner Complete. Elapsed " << sw2.elapsed<std::chrono::milliseconds>( ) << "ms\n";
		sw2.reset( );
		file_offset += _data_size;
	}
	sw.stop( );
	std::cout << "Complete. Elapsed " << sw.elapsed<std::chrono::microseconds>( ) << "us\n";
	std::cout << "Complete. Elapsed " << sw.elapsed<std::chrono::milliseconds>( ) << "ms\n";


	mmf.reset( nullptr );
	LARGE_INTEGER actual_size{ static_cast<DWORD>( file_offset ) };
	std::cout << "Resizing file to: " << file_offset << '\n';
	SetFilePointerEx( fhandle.get( ), actual_size, NULL, FILE_BEGIN );
	SetEndOfFile( fhandle.get( ) );
}

int wmain( int argc, wchar_t* argv[ ] )
{
	test_pointer( );
	test_known_path( );
	test_temp_path( );
	test_current_path( );
	if( argc > 1 )
	{
		std::wstring path{ argv[ 1 ] };
		test_create_dir( path );
		test_dir_exists( path );

		std::cout << '\n';
		std::cout << "Overlapped Write Test\n";
		test_write_overlapped( LR"(C:\Users\wbuckley\OneDrive\Test\DirTest\OverLapped.txt)" );

		std::cout << '\n';
		std::cout << "Overlapped Read Test\n";
		test_read_overlapped( LR"(C:\Users\wbuckley\OneDrive\Test\DirTest\OverLapped.txt)" );
		
		std::cout << '\n';
		std::cout << "MMF Test\n";
		test_mmf( LR"(C:\Users\wbuckley\OneDrive\Test\DirTest\MemoryMapped.txt)" );
		test_remove_dir( path );
	}
	test_encoding( ); 
	std::cin.get( );
	return 0;
}

