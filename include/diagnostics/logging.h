
#pragma once

// load the language-specific assertions implementation:

#if defined __cplusplus
#include <diagnostics/implementation/logging-cpp.h>
#else
#include <diagnostics/implementation/logging-C.h>
#endif
