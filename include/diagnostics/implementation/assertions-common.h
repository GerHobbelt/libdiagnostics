
#pragma once

#if !defined(LIBDIAG_NORETURN)
#if defined(__clang__) && defined(__has_attribute) && !defined(_MSC_VER)
#if __has_attribute(__noreturn__)
/* Clang defines both __clang__ and __GNUC__. Check __clang__ first. */
#  define LIBDIAG_NORETURN  __attribute__((__noreturn__))
#endif
#endif
#endif  // LIBDIAG_NORETURN
#if !defined(LIBDIAG_NORETURN)
#if (defined(__GNUC__) && (__GNUC__ >= 3)) || defined(__clang__) || defined(__IAR_SYSTEMS_ICC__)
#  define LIBDIAG_NORETURN  __attribute__((__noreturn__))
#elif defined(_MSC_VER)
#  define LIBDIAG_NORETURN  __declspec(noreturn)
#else
#  define LIBDIAG_NORETURN
#endif
#endif  // LIBDIAG_NORETURN



// When possible, define assert so that it does not add extra
// parentheses around EXPR.  Otherwise, those added parentheses would
// suppress warnings we'd expect to be detected by gcc's -Wparentheses.
#if defined __cplusplus 

#if !defined LIBDIAG_ASSERT_FILE
#  if defined __has_builtin
#   if __has_builtin(__builtin_FILE)
#    define LIBDIAG_ASSERT_FILE __builtin_FILE()
#    define LIBDIAG_ASSERT_LINE __builtin_LINE()
#   endif
#  endif
#endif
#if !defined LIBDIAG_ASSERT_FILE
#  define LIBDIAG_ASSERT_FILE __FILE__
#  define LIBDIAG_ASSERT_LINE __LINE__
#endif

#endif // __cplusplus

#if !defined LIBDIAG_ASSERT_C_FILE
// C does not support __builtin_FILE() et al, regrettably.
#  define LIBDIAG_ASSERT_C_FILE __FILE__
#  define LIBDIAG_ASSERT_C_LINE __LINE__
#endif

// Version 2.4 and later of GCC define a magical variable `__PRETTY_FUNCTION__'
// which contains the name of the function currently being defined.
// This is broken in G++ before version 2.6.
// C9x has a similar variable called __func__, but prefer the GCC one since
// it demangles C++ function names.
#if defined __cplusplus 

#if defined(__GNUC__) && (__GNUC__ >= 3)
#  define LIBDIAG_ASSERT_FUNCTION	__PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#  define LIBDIAG_ASSERT_FUNCTION	__FUNCSIG__
#else
#  define LIBDIAG_ASSERT_FUNCTION	__func__
#endif

#endif // __cplusplus

#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
#  define LIBDIAG_ASSERT_C_FUNCTION	__func__
#elif defined(_MSC_VER)
// Visual Studio 6 does not know __func__ or __FUNCTION__
// The rest of MS compilers use __FUNCTION__, not C99 __func__
#  define LIBDIAG_ASSERT_C_FUNCTION	__FUNCTION__
#else
#  define LIBDIAG_ASSERT_C_FUNCTION	"???"
#endif


// This macro is needed to help to remove: "warning C4003: not enough arguments for function-like
// macro invocation '<identifier>'" when expanding __VA_ARGS__. In our setup, MSVC treats this
// warning as an error. A sample code to reproduce the case: https://godbolt.org/z/M4zZNG.
#define LIBDIAG_PASS_ON(...) __VA_ARGS__

// This is a workaround for fact that MSVC expands __VA_ARGS__ after passing them into a macro,
// rather than before passing them into a macro. Without this, _ASSERT_SELECTOR does not work
// correctly when compiled with MSVC
#define LIBDIAG_EXPAND(X) X





#if defined(DIAG_NO_ASSERTIONS)
#define DIAG_ASSERTIONS_MODE 0
#endif
#if defined(NDEBUG) && !defined(DIAG_ASSERTIONS_MODE)
#define DIAG_ASSERTIONS_MODE 0
#endif
#if defined(_DEBUG) && !defined(DIAG_ASSERTIONS_MODE)
#define DIAG_ASSERTIONS_MODE 2
#endif
#if !defined(DIAG_ASSERTIONS_MODE)
#define DIAG_ASSERTIONS_MODE 1
#endif


