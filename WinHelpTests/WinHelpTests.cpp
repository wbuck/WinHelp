#include "fs_helper.hpp"
#include "encoding_conv.hpp"
#include "expected.hpp"
#include "flags.hpp"
#include "io_req.hpp"
#include "win_handle.hpp"
#include "custom_ptrs.hpp"
#include "stopwatch.hpp"
#include <variant>
#include <iostream>
#include <fstream>
#include <system_error>
#include <array>
#include <vector>
#include <fstream>
#include <tuple>

#ifndef DEBUG_BREAK
#	define DEBUG_BREAK __debugbreak( )
#endif



static constexpr DWORD _data_size{ 1728 * 400 };
std::vector<char> _buffer_a( _data_size, 'A' );
std::vector<char> _buffer_b( _data_size, 'B' );
std::vector<char> _buffer_c( _data_size, 'C' );
std::vector<char> _buffer_d( _data_size, 'D' );

static void test_known_path( )
{
	std::cout << "\nKnown path test\n";
	if( auto expected{ wh::fs::get_known_path( FOLDERID_CommonPrograms ) }; expected )
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
	if( auto expected{ wh::fs::get_temp_path( ) }; expected )
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
	if( auto expected{ wh::fs::get_current_dir( ) }; expected )
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
	if( auto expected{ wh::fs::create_dir( dir ) }; expected )
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
	if( wh::fs::dir_exists( dir ) )
		std::cout << "Directory exists test success\n";	
}

static void test_create_file( std::wstring_view filepath )
{
	std::cout << "\nCreate file test\n";
	if( auto expected{ wh::fs::create_file( 
		filepath, 
		wh::access_flag::read | wh::access_flag::write, 
		wh::share_flag::share_none,
		wh::creation_option::create_always, 
		wh::attr_flag::flag_delete_on_close ) }; expected )
	{		
		//wh::unique_file_ptr fhandle{ std::move( expected ).value( ) };
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
	if( auto expected{ wh::fs::remove_dir( dir ) }; expected )
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
	if( auto expected_utf8{ wh::conv::to_utf8( utf16str ) }; expected_utf8 )
	{
		utf8str = expected_utf8.value( );
		std::cout << "UTF16 to UTF8 conversion success: [" << utf8str << "]\n";

		std::cout << "\nUTF8 to UTF16 test\n";
		if( auto expected_utf16{ wh::conv::to_utf16( utf8str ) };  expected_utf16 )
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
	wh::unique_file_ptr null_handle{ nullptr };
	if( !null_handle ) std::cout << "Pointer [NULL] test success\n";
	else std::cout << "Pointer test failed: Invalid handle returned true\n";

	wh::unique_file_ptr invalid_handle{ INVALID_HANDLE_VALUE };
	if( !invalid_handle ) std::cout << "Pointer [INVALID] test success\n";
	else std::cout << "Pointer test failed: Invalid handle returned true\n";
}

template<typename T,
	typename = std::enable_if_t<std::is_unsigned_v<T>>>
	constexpr auto round_up( T num_to_round, T multiple )
{
	return ( ( num_to_round + multiple - 1 ) / multiple ) * multiple;
}

template<typename T,
	typename = std::enable_if_t<std::is_unsigned_v<T>>>
	constexpr auto round_down( T num_to_round, T multiple )
{
	return ( num_to_round / multiple ) * multiple;
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

static void test_write_nb_overlapped( std::wstring_view filepath )
{
	using namespace wh;
	using namespace wh::diag;
	
	auto fexpected{ wh::fs::create_file(
		filepath,
		access_flag::write,
		share_flag::share_none,
		creation_option::create_always,
		attr_flag::flag_no_buffering ) };
	
	if( !fexpected )
	{
		auto ec{ fexpected.error( ) };
		std::cout << "Failed to create file: " << ec.message( ) << '\n';
		return;
	}
	auto fhandle{ std::move( fexpected ).value( ) };

	LARGE_INTEGER file_size{ 0 };
	file_size.QuadPart = round_up( _data_size, 512UL );

	//if( !SetFilePointerEx( fhandle.get( ), file_size, nullptr, FILE_BEGIN ) )
	//{
	//	std::error_code ex{ static_cast<int32_t>( GetLastError( ) ), std::system_category( ) };
	//	std::cout << "Failed: SetFilePointer - " << ex.message( ) << '\n';
	//}
	//SetEndOfFile( fhandle.get( ) );
	
	// Allocate aligned memory.
	wh::unique_any_ptr<LPVOID, &_aligned_free> uptr{ _aligned_malloc( _data_size, 512 ) };
	if( !uptr )
	{
		std::cout << "Failed to allocate aligned memory\n";
		return;
	}
	std::uintptr_t* ptr{ reinterpret_cast<std::uintptr_t*>( uptr.get( ) ) };

	if( reinterpret_cast<std::uintptr_t>( uptr.get( ) ) % 512 == 0 )
		std::cout << "Memory correctly aligned\n";
	else 
		std::cout << "Failed to align memory\n";

	const uint64_t* out{ reinterpret_cast<uint64_t*>( _buffer_a.data( ) ) };
	const std::size_t size{ _buffer_a.size( ) / sizeof( uint64_t ) };

	std::copy( out, out + size, ptr );
	//uint64_t count{ 0 };
	//while( count < size )
	//{
	//	*ptr++ = *out++;
	//	++count;
	//}
	DWORD written{ 0 };
	stopwatch sw;
	sw.start( );
	if( !WriteFile( fhandle.get( ), _buffer_a.data( ), file_size.QuadPart, &written, nullptr ) )
	{
		std::error_code ex{ static_cast<int32_t>( GetLastError( ) ), std::system_category( ) };
		std::cout << "Failed: WriteFile - " << ex.message( ) << '\n';
		return;
	}
	sw.stop( );
	std::cout << "Complete. Elapsed " << sw.elapsed_microseconds( ) << "us\n";
	std::cout << "Complete. Elapsed " << sw.elapsed_milliseconds( ) << "ms\n";
	std::cout << "Bytes written " << written << '\n';
	//file_size.QuadPart = _data_size;
	//SetFilePointerEx( fhandle.get( ), file_size, nullptr, FILE_BEGIN );
	//SetEndOfFile( fhandle.get( ) );
}

static void test_write_overlapped( std::wstring_view filepath )
{
	using namespace wh;
	using namespace wh::diag;

	auto privexpected{ wh::util::set_privilege( SE_MANAGE_VOLUME_NAME, true ) };
	auto fexpected{ wh::fs::create_file(
		filepath,
		access_flag::write,
		share_flag::share_none,
		creation_option::create_always,
		attr_flag::flag_overlapped ) };

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
	file_size.QuadPart = _data_size * 4;

	SetFilePointerEx( fhandle.get( ), file_size, nullptr, FILE_BEGIN );
	SetEndOfFile( fhandle.get( ) );

	// We are only able to use the valid file data
	// function if setting the privilege was successful.
	if( privexpected )
	{
		if( !SetFileValidData( fhandle.get( ), file_size.QuadPart ) )
		{
			std::error_code ex{ static_cast<int32_t>( 
				GetLastError( ) ), std::system_category( ) };
		}
	}

	static constexpr int32_t count{ 4 };
	std::vector<wh::write_io_req<std::vector<char>>> requests;

	std::cout << "Creating requests\n";
	for( int32_t i{ 0 }; i < count; i++ )
		requests.emplace_back( _data_size * i );
	

	stopwatch sw{ };	
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
		else
			buffer = &_buffer_d;
		

		if( auto result{ req.write_async(
				fhandle.get( ), std::move( *buffer ) ) }; !result )
		{
			std::cout << result.error( ).message( ) << '\n';
			return;
		}
	}

	if( auto ec{ wh::wait_completion_req(
		requests, std::chrono::milliseconds{ INFINITE } ) }; !ec )
	{
		std::cerr << "Error waiting " << ec.error( ).message( ) << '\n';
	}
	sw.stop( );

	if( auto ec{ wh::remove_completed_req( fhandle.get( ), requests ) }; !ec )
		std::cerr << "Error writing " << ec.error( ).message( ) << '\n';

	std::cout << "Complete. Elapsed " << sw.elapsed_microseconds( ) << "us\n";
	std::cout << "Complete. Elapsed " << sw.elapsed_milliseconds( ) << "ms\n";
}

static void test_read_overlapped( std::wstring_view filepath )
{
	using namespace wh;
	using namespace wh::diag;

	auto fexpected{ wh::fs::create_file(
		filepath,
		access_flag::write | access_flag::read,
		share_flag::share_none,
		creation_option::open_existing,
		attr_flag::flag_overlapped ) };

	if( !fexpected )
	{
		auto ec{ fexpected.error( ) };
		std::cout << "Failed to create file: " << ec.message( ) << '\n';
		return;
	}
	auto fhandle{ std::move( fexpected ).value( ) };

	static constexpr int32_t count{ 4 };
	std::vector<wh::read_io_req> requests;

	std::cout << "Creating requests\n";
	for( int32_t i{ 0 }; i < count; i++ )
		requests.emplace_back( _data_size * i );

	
	stopwatch sw{ };
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
	if( auto ec{ wh::wait_completion_req(
		requests, std::chrono::milliseconds{ INFINITE } ) }; !ec )
	{
		std::cerr << "Error waiting " << ec.error( ).message( ) << '\n';
	}
	if( auto ec{ wh::remove_completed_req( fhandle.get( ), requests ) }; !ec )
		std::cerr << "Error writing " << ec.error( ).message( ) << '\n';

	sw.stop( );

	std::cout << "Complete. Elapsed " << sw.elapsed_microseconds( ) << "us\n";
	std::cout << "Complete. Elapsed " << sw.elapsed_milliseconds( ) << "ms\n";

}

static void test_overlapped( std::wstring_view filepath )
{
	using namespace wh;
	using namespace wh::diag;

	auto fexpected{ wh::fs::create_file( 
		filepath, 
		access_flag::write | access_flag::read, 
		share_flag::share_none, 
		creation_option::create_always, 
		attr_flag::flag_overlapped ) };

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
	stopwatch sw{ };
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

	std::cout << "Complete. Elapsed " << sw.elapsed_microseconds( ) << "us\n";
	std::cout << "Complete. Elapsed " << sw.elapsed_milliseconds( ) << "ms\n";
}

class system_info : public SYSTEM_INFO
{
public:
	system_info( ) noexcept { GetSystemInfo( this ); }
};


static wh::unique_view_ptr create_view2( 
	HANDLE mmf, std::size_t offset, std::size_t length )
{
	static const system_info info{ };
	static const std::size_t granularity{ info.dwAllocationGranularity };
	
	// Round the offset down to the nearest
	// multiple of the allocation granularity.
	const std::size_t corrected_offset{ round_down( offset, granularity ) };
	std::size_t correction{ offset - corrected_offset };
	// Increase the requested length to ensure
	// we pass a view which contains the requested
	// addresses.
	length += correction;

	const DWORD high{ static_cast<DWORD>( ( corrected_offset >> 32 ) & 0xFFFFFFFFUL ) };
	const DWORD low{ static_cast<DWORD>( corrected_offset & 0xFFFFFFFFUL ) };
	
	auto ptr{ static_cast<char*>( 
		MapViewOfFile( mmf, FILE_MAP_ALL_ACCESS, high, low, length ) ) };

	// Move the pointer to the offset position specied.
	if( ptr ) ptr += correction;

	return wh::unique_view_ptr{ ptr };
}

static void test_mmf( std::wstring_view filepath )
{
	using namespace wh;
	using namespace wh::fs;
	using namespace wh::diag;	
	
	auto fexpected{ wh::fs::create_file(
		filepath,
		access_flag::write | access_flag::read,
		share_flag::share_read | share_flag::share_write,
		creation_option::create_always,
		attr_flag::attr_normal ) };

	if( !fexpected )
	{
		auto ec{ fexpected.error( ) };
		std::cout << "Failed to create file: " << ec.message( ) << '\n';
		return;
	}
	auto fhandle{ std::move( fexpected.value( ) ) };

	static const system_info info{ };
	static const std::size_t granularity{ info.dwAllocationGranularity };

	const int64_t view_size{ round_up( _data_size, info.dwAllocationGranularity ) };
	const LARGE_INTEGER file_size{ view_size * 4 };

	SetFilePointerEx( fhandle.get( ), file_size, nullptr, FILE_BEGIN );
	SetEndOfFile( fhandle.get( ) );
	
	if( file_size.QuadPart % view_size != 0 )
	{
		std::cerr << "Invalid view size\n";
		return;
	}

	auto mmfexpected{ create_mapping( fhandle.get( ),
		protection_flag::read_write, 0 ) };

	if( !mmfexpected )
	{
		std::cout << "Failed to map file: " << mmfexpected.error( ).message( ) << '\n';
		return;
	}
	unique_file_ptr mmf{ std::move( mmfexpected ).value( ) };
	if( !mmf )
	{
		std::error_code ec{ static_cast<int32_t>(
			GetLastError( ) ), std::system_category( ) };
		std::cout << "Failed to create MMF: " << ec.message( ) << '\n';
		return;
	}

	static constexpr int32_t count{ 4 };
	std::array<char*, count> pointers{ };

	stopwatch sw{ };
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
		}

		auto viewexpected{ create_view( mmf.get( ), file_offset, _data_size ) };
		if( !viewexpected )
		{
			std::cout << "Failed to create MMF view: " << viewexpected.error( ).message( ) << '\n';
			return;
		}
		unique_view_ptr view{ std::move( viewexpected ).value( ) };
		char* outbuf{ static_cast<char*>( view.get( ) ) };		
		std::copy( pointers[ i ], pointers[ i ] + _data_size, outbuf );
		file_offset += _data_size;
	}
	sw.stop( );
	std::cout << "Complete. Elapsed " << sw.elapsed_microseconds( ) << "us\n";
	std::cout << "Complete. Elapsed " << sw.elapsed_milliseconds( ) << "ms\n";


	mmf.reset( nullptr );
	LARGE_INTEGER actual_size{ static_cast<DWORD>( file_offset ) };
	std::cout << "Resizing file to: " << file_offset << '\n';
	SetFilePointerEx( fhandle.get( ), actual_size, NULL, FILE_BEGIN );
	SetEndOfFile( fhandle.get( ) );
}

int wmain( int argc, wchar_t* argv[ ] )
{
	//test_pointer( );
	//test_known_path( );
	//test_temp_path( );
	//test_current_path( );
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

		//std::cout << '\n';
		//std::cout << "File stream Test\n";
		//test_file_write( R"(C:\Users\wbuckley\OneDrive\Test\DirTest\Stream.txt)" );
		//
		//std::cout << '\n';
		//std::cout << "MMF Test\n";
		//test_mmf( LR"(C:\Users\wbuckley\OneDrive\Test\DirTest\MemoryMapped.txt)" );
		test_remove_dir( path );
	}
	test_encoding( ); 
	std::cin.get( );
	return 0;
}

