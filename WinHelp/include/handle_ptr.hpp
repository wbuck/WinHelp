#pragma once
#include <memory>
#include <type_traits>

namespace wh
{
	template<auto fn>
	using deleter_from_fn = std::integral_constant<decltype( fn ), fn>;

	template<typename T, auto fn>
	using unique_handle_ptr = std::unique_ptr<typename std::remove_pointer_t<T>, deleter_from_fn<fn>>;
}
