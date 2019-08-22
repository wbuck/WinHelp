#include "pch.h"
#include "fs_helper.hpp"
#include "custom_ptrs.hpp"
#include "encoding_conv.hpp"
#include "scoped_exit.hpp"
#include "win_utility.hpp"
#include <vector>
#include <system_error>
#include <Shellapi.h>
#include <Shobjidl.h>
#include <shlobj.h>

namespace wh::fs
{
	expected_ec_t<std::wstring> get_known_path( const KNOWNFOLDERID& id ) noexcept
	{
		PWSTR buffer{ nullptr };
		HRESULT hresult{ SHGetKnownFolderPath( id, KF_FLAG_DEFAULT, nullptr, &buffer ) };

		if( SUCCEEDED( hresult ) )
		{
			unique_any_ptr<PWSTR, &CoTaskMemFree> pbuffer{ buffer };
			return std::wstring{ pbuffer.get( ) };
		}
		return std::error_code{ static_cast<int32_t>( hresult ), 
			std::system_category( ) };
	}

	expected_ec_t<std::wstring> get_temp_path( ) noexcept
	{
		static constexpr DWORD initial_size{ MAX_PATH + 1 };
		DWORD copied{ initial_size };
		std::wstring collapsed_path( initial_size, L'\0' );
				
		while( true )
		{
			// The get 'short' temp path.
			copied = GetTempPathW( copied, collapsed_path.data( ) );

			// If the value of copied is greater than
			// our buffer, we need to resize the buffer
			// and try again.
			if( copied > static_cast<DWORD>( collapsed_path.size( ) ) )
				collapsed_path.resize( copied );

			else if( copied == 0 )
			{
				return std::error_code{ static_cast<int32_t>( GetLastError( ) ),
					std::system_category( ) };
			}
			else break;
		}

		std::wstring full_path( initial_size, L'\0' );
		copied = initial_size;

		while( true )
		{
			// Get the 'long' path name from the 
			// supplied 'short' path.
			copied = GetLongPathNameW( collapsed_path.c_str( ), 
				full_path.data( ), copied );

			if( copied > static_cast<DWORD>( full_path.size( ) ) )
				full_path.resize( copied );

			else if( copied == 0 )
			{
				return std::error_code{ static_cast<int32_t>( GetLastError( ) ),
					std::system_category( ) };
			}
			else
			{
				full_path.resize( copied );
				break;
			}
		}
		return full_path;
	}

	expected_ec_t<void> create_dir( std::wstring_view dir ) noexcept
	{
		if( CreateDirectoryW( dir.data( ), nullptr ) )
			return { };
		
		return std::error_code{ static_cast<int32_t>( GetLastError( ) ),
			std::system_category( ) };
	}

	expected_ec_t<std::wstring> get_current_dir( ) noexcept
	{
		// Determine the required size of the buffer.
		DWORD size{ GetCurrentDirectoryW( 0, nullptr ) };
		std::wstring buffer( size, L'\0' );

		if( DWORD copied{ GetCurrentDirectoryW( size, buffer.data( ) ) }; copied == 0 )
		{
			return std::error_code{ static_cast<int32_t>( GetLastError( ) ),
				std::system_category( ) };
		}	
		else
		{
			buffer.resize( copied );
			return buffer;
		}		
	}

	expected_ec_t<void> remove_dir( std::wstring_view dir ) noexcept
	{
		// Lambda used as a deleter for the unique ptr.
		static constexpr auto release =
			[ ]( auto obj ) -> void { if( obj ) obj->Release( ); };

		using file_op_ptr = std::unique_ptr<IFileOperation, decltype( release )>;
		using item_ptr = std::unique_ptr<IShellItem, decltype( release )>;

		HRESULT hresult{ CoInitializeEx( nullptr,
			COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE ) };

		if( FAILED( hresult ) )
			return std::error_code{ hresult, std::system_category( ) };

		// Unitializes the COM object when the scope exits.
		const auto cleanup = on_scoped_exit( [ ] { CoUninitialize( ); } );

		// Lambda for creating and initializing an
		// IFileOperation object.
		const auto create_file_operation = [ ]( REFIID riid, void** ppv )
		{
			*ppv = nullptr;

			// Create the IFileOperation object.
			IFileOperation* file_op{ nullptr };
			HRESULT hresult{ CoCreateInstance( __uuidof( FileOperation ),
				nullptr, CLSCTX_ALL, IID_PPV_ARGS( &file_op ) ) };

			file_op_ptr pfile_op{ file_op, release };
			if( SUCCEEDED( hresult ) )
			{
				// Set the operation flags. Turn off all UI
				// from being shown to the user during the
				// operation. This includes error, confirmation
				// and progress dialogs.
				hresult = pfile_op->SetOperationFlags( FOF_NO_UI );

				if( SUCCEEDED( hresult ) )
					pfile_op->QueryInterface( riid, ppv );
			}
			return hresult;
		};

		IFileOperation* file_op{ nullptr };
		hresult = create_file_operation( IID_PPV_ARGS( &file_op ) );
		file_op_ptr pfile_op{ file_op, release };

		if( FAILED( hresult ) )
			return std::error_code{ hresult, std::system_category( ) };

		// Get array of all items in the supplied directory
		// which needs to be recursively deleted.
		IShellItem* item{ nullptr };
		hresult = SHCreateItemFromParsingName(
			dir.data( ), nullptr, IID_PPV_ARGS( &item ) );

		item_ptr pitem{ item, release };

		if( FAILED( hresult ) )
			return std::error_code{ hresult, std::system_category( ) };

		// Mark item for deletion.
		hresult = pfile_op->DeleteItem( item, nullptr );

		if( FAILED( hresult ) )
			return std::error_code{ hresult, std::system_category( ) };

		// Delete files/folders which had been
		// marked for deletion.
		hresult = pfile_op->PerformOperations( );

		if( FAILED( hresult ) )
			return std::error_code{ hresult, std::system_category( ) };

		return { };
	}

	bool dir_exists( std::wstring_view dir ) noexcept
	{		
		if( DWORD attributes{ GetFileAttributesW( dir.data( ) ) }; 
			attributes == INVALID_FILE_ATTRIBUTES )
			return false;
		else
			return ( attributes & FILE_ATTRIBUTE_DIRECTORY );
	}

	expected_ec_t<unique_file_ptr> create_file( std::wstring_view filename,
											   access_flag access, 
											   share_flag mode, 
											   creation_option options, 
											   attr_flag flags ) noexcept
	{
		unique_file_ptr phandle{ 
			CreateFileW( filename.data( ),
						 static_cast<DWORD>( access ),
						 static_cast<DWORD>( mode ),
						 nullptr,
						 static_cast<DWORD>( options ),
						 static_cast<DWORD>( flags ),
						 nullptr ) };
		if( !phandle )
		{
			return std::error_code{ static_cast<int32_t>( GetLastError( ) ),
				std::system_category( ) };
		}		
		return phandle;
	}

	expected_ec_t<unique_file_ptr> create_mapping( HANDLE file,
												   protection_flag protect, 
												   std::size_t size, 
												   const wchar_t* name ) noexcept
	{
		auto const [ high, low ] = util::split_size( size );

		unique_file_ptr mmf{ CreateFileMappingW( file,
			nullptr, static_cast<DWORD>( protect ),
			high, low, name ) };

		if( !mmf )
		{
			return std::error_code{ static_cast<int32_t>(
				GetLastError( ) ), std::system_category( ) };
		}
		return mmf;
	}

	struct system_info : public SYSTEM_INFO
	{ system_info( ) noexcept { GetSystemInfo( this ); } };

	expected_ec_t<unique_view_ptr> create_view( HANDLE mmf, 
												std::size_t offset, 
												std::size_t size ) noexcept
	{
		static const system_info info{ };
		static const std::size_t granularity{ info.dwAllocationGranularity };

		// Round the offset down to the nearest
		// multiple of the allocation granularity.
		const std::size_t corrected_offset{ util::round_down( offset, granularity ) };
		std::size_t correction{ offset - corrected_offset };

		// Increase the requested length to ensure
		// we pass a view which contains the requested
		// addresses.
		size += correction;

		auto const [ high, low ] = util::split_size( corrected_offset );

		LPVOID view{ MapViewOfFile( mmf, FILE_MAP_ALL_ACCESS, high, low, size ) };
		if( !view )
		{
			return std::error_code{ static_cast<int32_t>( GetLastError( ) ),
				std::system_category( ) };
		}
		// Move the pointer to the offset position specied.
		return wh::unique_view_ptr{ static_cast<char*>( view ) + correction };
	}


}