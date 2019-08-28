#pragma once

#include "custom_ptrs.hpp"
#include "encoding_conv.hpp"
#include "expected.hpp"
#include "flags.hpp"
#include "fs_helper.hpp"
#include "io_req.hpp"
#include "scoped_exit.hpp"
#include "stopwatch.hpp"
#include "win_handle.hpp"
#include "win_utility.hpp"

namespace winh
{	
	inline namespace conv { using namespace ::wh::conv; }
	inline namespace diag { using namespace ::wh::diag; }
	inline namespace fs { using namespace ::wh::fs; }
	inline namespace util { using namespace ::wh::util; }
	inline namespace wh { using namespace ::wh; }
}
