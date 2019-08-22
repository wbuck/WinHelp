#pragma once
#include "win_handle.hpp"
#include <windows.h>
#include <memory>
#include <type_traits>


namespace wh
{
	template<auto fn>
	using deleter_from_fn = std::integral_constant<decltype( fn ), fn>;

	template<typename T, auto fn>
	using unique_any_ptr =
		std::unique_ptr<typename std::remove_pointer_t<T>, deleter_from_fn<fn>>;

	using unique_file_ptr = std::unique_ptr<
		win_handle<>, win_handle<>::deleter>;

	using unique_view_ptr = unique_any_ptr<LPVOID, &UnmapViewOfFile>;
	using unique_event_ptr = unique_any_ptr<HANDLE, &CloseHandle>;
}


