#pragma once
#include "expected.hpp"
#include "custom_ptrs.hpp"
#include "win_utility.hpp"
#include <windows.h>
#include <type_traits>
#include <system_error>
#include <chrono>
#include <algorithm>
#include <vector>

namespace wh
{
	template<typename T, typename = void>
	struct is_container : public std::false_type { };

	template<typename T>
	struct is_container<T, 
		std::void_t<decltype( std::declval<T>( ).data( ) ), 
					decltype( std::declval<T>( ).size( ) )>> 
		: public std::true_type { };

	template<typename T, typename = void>
	struct is_io_req : public std::false_type { };

	template<typename T>
	struct is_io_req <T, std::void_t<decltype( std::declval<T>( ).get_event( ) )>> 
		: public std::true_type{ };

	template<typename T>
	inline constexpr bool is_container_v = is_container<T>::value;

	template<typename T>
	inline constexpr bool is_io_req_v = is_io_req<T>::value;

	template<typename Container>
	class write_io_req final : public OVERLAPPED
	{		
		static_assert( is_container_v<Container>, 
					   "Container must contain both 'data' and 'size' member functions" );
	public:
		using container_type = Container;
		using container_value_type = typename Container::value_type;

		write_io_req( std::size_t offset ) noexcept;
			
		operator const HANDLE( ) const noexcept;

		expected_ec_t<void> write_async( HANDLE file, const Container& data )
			noexcept( std::is_nothrow_copy_assignable_v<Container> );

		expected_ec_t<void> write_async( HANDLE file, Container&& data )
			noexcept( std::is_nothrow_move_assignable_v<Container> );

		const HANDLE get_event( ) const noexcept;

	private:

		expected_ec_t<void> write( HANDLE file ) noexcept;

		expected_ec_t<void> create_event( ) noexcept;

		unique_event_ptr _event;
		Container _buffer;
	};

	template<typename Container>
	inline write_io_req<Container>::write_io_req( std::size_t offset ) noexcept
		: OVERLAPPED{ }
	{
		auto [ high, low ] { util::split_size( offset ) };
		OffsetHigh = high;
		Offset = low;
		hEvent = nullptr;
	} 

	template<typename Container>
	inline write_io_req<Container>::operator const HANDLE( ) const noexcept
	{ return get_event( ); }

	template<typename Container>
	inline expected_ec_t<void> write_io_req<Container>::write_async( HANDLE file, const Container& data ) 
		noexcept( std::is_nothrow_copy_assignable_v<Container> )
	{
		_buffer = data;
		return write( file );
	}

	template<typename Container>
	inline expected_ec_t<void> write_io_req<Container>::write_async( HANDLE file, Container&& data ) 
		noexcept( std::is_nothrow_move_assignable_v<Container> )
	{
		_buffer = std::move( data );
		return write( file );
	}

	template<typename Container>
	inline const HANDLE write_io_req<Container>::get_event( ) const noexcept
	{
		return _event.get( );
	}

	template<typename Container>
	inline expected_ec_t<void> write_io_req<Container>::write( HANDLE file ) noexcept
	{
		if( auto result{ create_event( ) }; !result )
			return result;

		if( !WriteFile( file, _buffer.data( ), 
				static_cast<DWORD>( _buffer.size( ) ), nullptr, this ) )
		{
			if( auto code{ GetLastError( ) }; code != ERROR_IO_PENDING )
				return util::get_errorcode( code );			
		}
		return { };
	}

	template<typename Container>
	inline expected_ec_t<void> write_io_req<Container>::create_event( ) noexcept
	{
		if( !hEvent )
		{
			if( auto expected{ util::create_event( true, false ) }; !expected )
				return std::move( expected ).error( );
			else
			{
				_event = std::move( expected ).value( );
				hEvent = _event.get( );
			}
		}
		return { };
	}


	class read_io_req final : public OVERLAPPED
	{
	public:
		read_io_req( std::size_t offset ) noexcept;

		operator const HANDLE( ) const noexcept
		{ return get_event( ); }

		template<typename Ptr>
		expected_ec_t<void> read_async( HANDLE file, 
			const Ptr buf, std::size_t count ) noexcept;

		const HANDLE get_event( ) const noexcept;

	private:
		expected_ec_t<void> create_event( ) noexcept;

		unique_event_ptr _event;
	};

	inline read_io_req::read_io_req( std::size_t offset ) noexcept
		: OVERLAPPED{ }
	{
		auto [ high, low ] { util::split_size( offset ) };
		OffsetHigh = high;
		Offset = low;
		hEvent = nullptr;
	}

	inline const HANDLE read_io_req::get_event( ) const noexcept
	{ return _event.get( ); }

	inline expected_ec_t<void> read_io_req::create_event( ) noexcept
	{
		if( !hEvent )
		{
			if( auto expected{ util::create_event( true, false ) }; !expected )
				return std::move( expected ).error( );
			else
			{
				_event = std::move( expected ).value( );
				hEvent = _event.get( );
			}
		}
		return { };
	}

	template<typename Ptr>
	inline expected_ec_t<void> read_io_req::read_async( 
		HANDLE file, const Ptr buf, std::size_t count ) noexcept
	{
		static_assert( std::is_pointer_v<Ptr>, "Paramater 'buf' must be pointer type" );
		if( auto result{ create_event( ) }; !result )
			return result;

		if( !ReadFile( file, buf, 
					   static_cast<DWORD>( count ), nullptr, this ) )
		{
			if( auto code{ GetLastError( ) }; code != ERROR_IO_PENDING )
				return util::get_errorcode( code );			
		}
		return { };
	}

	template<typename Container>
	inline expected_ec_t<void> wait_completion_req( const Container& cont,
		std::chrono::milliseconds timeout ) noexcept
	{
		static_assert( is_io_req_v<typename Container::value_type>, "Invalid type" );
		static_assert( std::is_base_of_v<OVERLAPPED, typename Container::value_type>, "Invalid type" );

		if( auto count{ static_cast<DWORD>( cont.size( ) ) }; count > 0 )
		{
			std::vector<HANDLE> events;
			std::transform( std::begin( cont ),
				std::end( cont ), std::back_inserter( events ), 
				[ ]( const auto& req ) -> HANDLE { return req.get_event( ); } );

			if( auto result{ WaitForMultipleObjects(
				count, events.data( ), true, static_cast<DWORD>( timeout.count( ) ) ) }; 
				result == WAIT_FAILED )
			{
				return util::get_errorcode( GetLastError( ) );
			}
			else if( result == WAIT_TIMEOUT )
				return util::get_errorcode( WAIT_TIMEOUT );
		}
		return { };
	}

	template<typename Container>
	inline expected_ec_t<void> remove_completed_req( HANDLE file, Container& cont ) noexcept
	{		
		static_assert( is_io_req_v<typename Container::value_type>, "Invalid type" );		
		static_assert( std::is_base_of_v<OVERLAPPED, typename Container::value_type>, "Invalid type" );

		static constexpr DWORD no_wait{ 0 };
		DWORD ignored{ 0 };

		for( auto it{ std::begin( cont ) }; it != std::end( cont ); )
		{
			// Event is in the signaled state, check the
			// overlapped structure to determine the if
			// operation was successful.			
			if( GetOverlappedResult( file, &( *it ), &ignored, false ) )
				it = cont.erase( it );
			
			else if( DWORD code{ GetLastError( ) }; code != ERROR_IO_INCOMPLETE )
				return util::get_errorcode( code );
			
			else ++it;	
		}
		return { };
	}

}

