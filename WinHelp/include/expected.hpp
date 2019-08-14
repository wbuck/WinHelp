#pragma once
#include <variant>
#include <system_error>
#include <type_traits>

namespace wh
{
	// Type for representing a void return.
	struct void_t { };
	static constexpr void_t void_type{ };

	template<typename T, typename E>
	struct expected
	{
		static_assert( !std::is_same_v<T, E>, "T and E cannot be the same type" );

		constexpr expected( ) = delete;
		constexpr expected( const expected& rhs ) = default;
		constexpr expected( expected&& rhs ) = default;
		expected& operator=( const expected& rhs ) = default;
		expected& operator=( expected&& rhs ) = default;

		constexpr expected( T&& t ) noexcept
			: _value{ std::forward<T>( t ) } { }

		constexpr expected( E&& e ) noexcept
			: _value{ std::forward<E>( e ) } { }

		constexpr operator bool( ) const noexcept
		{
			return std::holds_alternative<T>( _value );
		}

		constexpr const T& value( ) const & { return std::get<T>( _value ); }
		constexpr T& value( ) & { return std::get<T>( _value ); }
		constexpr T&& value( ) && { return std::move( std::get<T>( _value ) ); }
		constexpr const T&& value( ) const && { return std::move( std::get<T>( _value ) ); }

		constexpr const E& error( ) const & { return std::get<E>( _value ); }
		constexpr E& error( ) & { return std::get<E>( _value ); }
		constexpr E&& error( ) && { return std::move( std::get<E>( _value ) ); }
		constexpr const E&& error( ) const && { return std::move( std::get<E>( _value ) ); }

	private:
		std::variant<std::monostate, T, E> _value;
	};

	template<typename E>
	struct expected<void, E>
	{
		expected( ) noexcept
			: _value{ void_type } { }

		expected( E&& e ) noexcept
			: _value{ std::forward<E>( e ) } { }


		constexpr operator bool( ) const noexcept
		{
			return std::holds_alternative<void_t>( _value );
		}

		constexpr const E& error( ) const & { return std::get<E>( _value ); }
		constexpr E& error( ) & { return std::get<E>( _value ); }
		constexpr E&& error( ) && { return std::move( std::get<E>( _value ) ); }
		constexpr const E&& error( ) const && { return std::move( std::get<E>( _value ) ); }

	private:
		std::variant<void_t, E> _value;
	};
}
