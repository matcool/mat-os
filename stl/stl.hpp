#pragma once

// name for the stl namespace
#define STL_NS stl
#define STL_NS_IMPL impl_detail

namespace STL_NS {}

#if USE_STL_NAMESPACE
using namespace STL_NS;
#endif