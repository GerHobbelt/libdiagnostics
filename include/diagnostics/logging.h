
#pragma once

// load the language-specific assertions implementation:

#if defined __cplusplus
#include <diagnostics/implementation/logging-cpp.h>
#else
#include <diagnostics/implementation/logging-C.h>
#endif


/*
* Logging statements should be easily and quickly filteraable (enable/disable output) at both compile time (debug logging ~~> debug diagnostics) and run time (program execution diagnostics).
*
* Also, these statements should be easily comprehensible at the source code level, hence formatting using either {fmt} or ostream operators is preferred.
*
* For easiest postprocessing outside the diagnostics-producing application itself, e.g. by using `grep`, implies a text format log file output, but we are also considering
* using binary and database (SQLite) output formats, as those might be more useful when we wish to store & postprocess large (compressed?!) log output volumes.
*
* To aid high performance run-time filtering, we use macros which generate a check expression (do we need to execute this diagnostics statemetn or can we skip/ignore it now?),
* followed by a stream-style setup expression, separated by `&&` boolean short-circuit evaluation.
* As C does not supports C++ streams and their `operator <<` syntax, we provide an alternative approach as well, which mimicks a function call alike approach using vararg preprocessor macros.
*
* Underneath it all, we expect to use `{fmt}` and `spdlog`.
*
* When logging image data and other BLOB-style data as part of our logging/diagnostics statements, those parts will be saved to separate files while the underlying libdiagnostics
* library code will produce customizable url-format reference paths to those files for including in the actual text-based log/diagnostics output itself.
*
*/
