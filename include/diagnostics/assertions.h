
#pragma once

// load the language-specific assertions implementation:

#if defined __cplusplus
#include <diagnostics/implementation/assertions-cpp.h>
#else
#include <diagnostics/implementation/assertions-C.h>
#endif
