
#pragma once

// load the language-specific assertions implementation:

#if defined __cplusplus
#include <diagnostics/implementation/diagnostics-cpp.h>
#else
#include <diagnostics/implementation/diagnostics-C.h>
#endif
