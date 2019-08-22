#pragma once
#include <windows.h>
#include <type_traits>
#include <iostream>

namespace wh
{	
	template<auto fn = &CloseHandle,
			 typename THandle = HANDLE>
	class win_handle
	{
	public:
		win_handle( std::nullptr_t = nullptr ) 
			: _handle{ nullptr } { }

		win_handle( THandle handle )
			: _handle{ handle == INVALID_HANDLE_VALUE ? nullptr : handle } { }

		explicit operator bool( ) const { return _handle != nullptr; }
		operator THandle( ) const { return _handle; }

		friend bool operator==( const win_handle& lhs, const win_handle& rhs ) 
		{ return lhs._handle == rhs._handle;  }

		friend bool operator!=( const win_handle& lhs, const win_handle& rhs )
		{ return !( lhs == rhs ); }

		struct deleter
		{
			using pointer = win_handle<fn, THandle>;
			void operator( )( pointer handle ) const 
			{
				if( handle ) std::invoke( fn, handle ); 
			}
		};

	private:
		THandle _handle;
	};

	template<auto fn = &CloseHandle,
		     typename THandle = HANDLE>
	inline bool operator==( const THandle& lhs, const win_handle<fn>& rhs )
	{ return lhs == rhs._handle; }

	template<auto fn = &CloseHandle,
			 typename THandle = HANDLE>
	inline bool operator!=( const THandle& lhs, const win_handle<fn>& rhs )
	{ return !( lhs == rhs ); }

	template<auto fn = &CloseHandle,
			 typename THandle = HANDLE>
	inline bool operator==( const win_handle<fn>& lhs, const THandle& rhs )
	{ return lhs._handle == rhs; }

	template<auto fn = &CloseHandle,
			 typename THandle = HANDLE>
	inline bool operator!=( const win_handle<fn>& lhs, const THandle& rhs )
	{ return !( lhs == rhs ); }
}
