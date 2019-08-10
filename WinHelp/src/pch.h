// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

#include "framework.h"
#include <windows.h>
#include <memory>
#include <type_traits>
#include <tchar.h>
#include <optional>
#include <string>

#ifndef WHNOTHROW
#include <exception>
#include <system_error>
#endif // WHNOTHROW


#endif //PCH_H