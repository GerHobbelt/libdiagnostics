
#pragma once

// load the language-specific assertions implementation:

#if defined __cplusplus
#include <diagnostics/implementation/telemetry-cpp.h>
#else
#include <diagnostics/implementation/telemetry-C.h>
#endif
