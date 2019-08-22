#pragma once
#include <type_traits>
#include <utility>

namespace wh
{
	template<typename Func>
	class scoped_exit_t final
	{
	public:

		explicit scoped_exit_t( Func&& fn )
			noexcept( std::is_nothrow_move_constructible_v<Func> )
			: _fn{ std::move( fn ) } { }

		scoped_exit_t( scoped_exit_t&& other )
			noexcept( std::is_nothrow_move_constructible_v<Func> )
			: _fn{ std::move( other._fn ) } { }
		
		~scoped_exit_t( ) noexcept( std::is_nothrow_invocable_v<Func> )
		{
			std::invoke( _fn );
		}

		scoped_exit_t( scoped_exit_t<Func> const& ) = delete;
		scoped_exit_t& operator=( scoped_exit_t<Func> const& ) = delete;
		scoped_exit_t& operator=( scoped_exit_t<Func>&& ) = delete;
	private:
		Func _fn;
	};

	template<typename Func>
	auto on_scoped_exit( Func&& fn )
		noexcept( std::is_nothrow_move_constructible_v<Func> )
	{		
		return scoped_exit_t<Func>{ std::move( fn ) };
	}
}
