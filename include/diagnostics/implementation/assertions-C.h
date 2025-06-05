
#pragma once

#include <assert.h>
#include <stdint.h>

#if defined(_WIN32)
#include <winsock2.h>
#include <windows.h>
#endif


#include <diagnostics/implementation/assertions-common.h>



# define LIBDIAG_ASSERT_VOID_CAST (void)




#if defined __cplusplus
extern "C" {
#endif

/* This prints an "Assertion failed" message and aborts / throws.  */
void LIBDIAG_NORETURN libdiag_assert_fail(const char *assertion, const char *file, unsigned int line, const char *function, const char *extra_message);

/* Likewise, but prints the error text for ERRNUM.  */
void LIBDIAG_NORETURN libdiag_perror_fail(int errnum, const char *assertion, const char *file, unsigned int line, const char *function, const char *extra_message);

#if defined(_WIN32)

/* Likewise, but prints the error text for ERRNUM.  */
void LIBDIAG_NORETURN libdiag_Win32error_fail(HRESULT errnum, const char *assertion, const char *file, unsigned int line, const char *function, const char *extra_message);

#endif




#if DIAG_ASSERTIONS_MODE == 0

#define DIAG_ASSERT(expr)		(LIBDIAG_ASSERT_VOID_CAST (0))

#else // DIAG_ASSERTIONS_MODE








// assertion macro mapping for 1 and 2 arguments: stolen from Intel Envoy (https://github.com/envoyproxy/envoy)

// CONDITION_STR is needed to prevent macros in condition from being expected, which obfuscates
// the logged failure, e.g., "EAGAIN" vs "11".
#define LIBDIAG_ASSERT_IMPL(CONDITION, CONDITION_STR, ACTION, DETAILS)                                    \
  do {                                                                                             \
      if (!!(CONDITION))								\
        ; /* empty */							\
      else								\
	  {                                                                            \
          libdiag_assert_fail(CONDITION_STR, LIBDIAG_ASSERT_FILE, LIBDIAG_ASSERT_LINE, LIBDIAG_ASSERT_FUNCTION, (DETAILS));	\
		  ACTION;                                                                                      \
      }                                                                                              \
  } while (0)

#if 0

#if !defined(NDEBUG) // If this is a debug build.
#define LIBDIAG_ASSERT_ACTION ::abort()
#else // If this is not a debug build, but ENVOY_LOG_(FAST)_DEBUG_ASSERT_IN_RELEASE is defined.
#define LIBDIAG_ASSERT_ACTION                                                                              \
  Envoy::Assert::invokeDebugAssertionFailureRecordActionForAssertMacroUseOnly(                     \
      __FILE__ ":" TOSTRING(__LINE__))
#endif // !defined(NDEBUG)

#else

#define LIBDIAG_ASSERT_ACTION  ((void)0)

#endif

#define LIBDIAG_ASSERT_ORIGINAL(X) LIBDIAG_ASSERT_IMPL(X, #X, LIBDIAG_ASSERT_ACTION, "")
#define LIBDIAG_ASSERT_VERBOSE(X, Y) LIBDIAG_ASSERT_IMPL(X, #X, LIBDIAG_ASSERT_ACTION, Y)
#define LIBDIAG_ASSERT_SELECTOR(_1, _2, ASSERT_MACRO, ...) ASSERT_MACRO

// If ASSERT is called with one argument, the ASSERT_SELECTOR will return
// _ASSERT_ORIGINAL and this will call _ASSERT_ORIGINAL(__VA_ARGS__).
// If ASSERT is called with two arguments, ASSERT_SELECTOR will return
// _ASSERT_VERBOSE, and this will call _ASSERT_VERBOSE,(__VA_ARGS__)
#define DIAG_ASSERT(...)                                                                                \
  LIBDIAG_EXPAND(LIBDIAG_ASSERT_SELECTOR(__VA_ARGS__, LIBDIAG_ASSERT_VERBOSE, LIBDIAG_ASSERT_ORIGINAL)(__VA_ARGS__))


// Asserts multiple expressions at once. Concept ripped from https://www.scs.stanford.edu/~dm/blog/va-opt.html.
#define PARENS ()

#define EXPAND(...) EXPAND4(EXPAND4(EXPAND4(EXPAND4(__VA_ARGS__))))
#define EXPAND4(...) EXPAND3(EXPAND3(EXPAND3(EXPAND3(__VA_ARGS__))))
#define EXPAND3(...) EXPAND2(EXPAND2(EXPAND2(EXPAND2(__VA_ARGS__))))
#define EXPAND2(...) EXPAND1(EXPAND1(EXPAND1(EXPAND1(__VA_ARGS__))))
#define EXPAND1(...) __VA_ARGS__

#define FOR_EACH(macro, ...)                                    \
  __VA_OPT__(EXPAND(FOR_EACH_HELPER(macro, __VA_ARGS__)))
#define FOR_EACH_HELPER(macro, a1, ...)                         \
  macro(a1)                                                     \
  __VA_OPT__(FOR_EACH_AGAIN PARENS (macro, __VA_ARGS__))
#define FOR_EACH_AGAIN() FOR_EACH_HELPER

// FOR_EACH(DIAG_ASSERT, a, b, c, 1, 2, 3)   // => F(a) F(b) F(c) F(1) F(2) F(3)

#define DIAG_ASSERT_FOR_EACH(...)                                                                  \
    do {                                                                                           \
		FOR_EACH(; DIAG_ASSERT, __VA_ARGS__);														   \
    } while (0)





// This non-implementation ensures that its argument is a valid expression that can be statically
// casted to a bool, but the expression is never evaluated and will be compiled away.
#define LIBDIAG_NULL_ASSERT_IMPL(X, ...)                                                           \
  do {                                                                                             \
    const int __assert_dummy_variable = 0 && !!(X);                        \
    (void)__assert_dummy_variable;                                                                 \
  } while (0)






/* This prints an "Assertion failed" message and aborts / throws.  */
void LIBDIAG_NORETURN libdiag_assert_fail_op(const int64_t a, const int64_t b, const char *assertion_lside, const char *assertion_operand, const char *assertion_rside, const char *operand_name, const char *file, unsigned int line, const char *function, const char *extra_message);


// CONDITION_STR is needed to prevent macros in condition from being expected, which obfuscates
// the logged failure, e.g., "EAGAIN" vs "11".
#define LIBDIAG_ASSERT_OP_IMPL(A, OP, B, A_STR, OP_STR, B_STR, OPERATOR_NAME, ACTION, DETAILS)                                    \
  do {                                                                                             \
	  int64_t a = (A);    \
	  int64_t b = (B);    \
      if (!!(a OP b))								\
        ; /* empty */							\
      else								\
	  {                                                                            \
          libdiag_assert_fail_op(a, b, A_STR, OP_STR, B_STR, #OPERATOR_NAME, LIBDIAG_ASSERT_FILE, LIBDIAG_ASSERT_LINE, LIBDIAG_ASSERT_FUNCTION, (DETAILS));	\
		  ACTION;                                                                                      \
      }                                                                                              \
  } while (0)

#define LIBDIAG_ASSERT_OP_ORIGINAL(A, OP, B, OPERATOR_NAME)   LIBDIAG_ASSERT_OP_IMPL(A, OP, B, #A, #OP, #B, OPERATOR_NAME, LIBDIAG_ASSERT_ACTION, "")
#define LIBDIAG_ASSERT_OP_VERBOSE(A, OP, B, OPERATOR_NAME, Y) LIBDIAG_ASSERT_OP_IMPL(A, OP, B, #A, #OP, #B, OPERATOR_NAME, LIBDIAG_ASSERT_ACTION, Y)
#define LIBDIAG_ASSERT_SELECTOR(_1, _2, ASSERT_MACRO, ...) ASSERT_MACRO


#define DIAG_ASSERT_OP(A, OP, B, OPERATOR_NAME, ...)                                                                                \
  LIBDIAG_EXPAND(LIBDIAG_ASSERT_SELECTOR(__VA_ARGS__, LIBDIAG_ASSERT_OP_VERBOSE, LIBDIAG_ASSERT_OP_ORIGINAL)(A, OP, B, OPERATOR_NAME __VA_OPT__(,) __VA_ARGS__))







#define DIAG_ASSERT_EQ(A, B, ...)                                                                                \
	DIAG_ASSERT_OP(A, ==, B, eq, __VA_ARGS__)

#define DIAG_ASSERT_NE(A, B, ...)                                                                                \
	DIAG_ASSERT_OP(A, !=, B, ne, __VA_ARGS__)

#define DIAG_ASSERT_GT(A, B, ...)                                                                                \
	DIAG_ASSERT_OP(A, >, B, gt, __VA_ARGS__)

#define DIAG_ASSERT_GE(A, B, ...)                                                                                \
	DIAG_ASSERT_OP(A, >=, B, ge, __VA_ARGS__)

#define DIAG_ASSERT_LE(A, B, ...)                                                                                \
	DIAG_ASSERT_OP(A, <=, B, le, __VA_ARGS__)

#define DIAG_ASSERT_LT(A, B, ...)                                                                                \
	DIAG_ASSERT_OP(A, <, B, lt, __VA_ARGS__)

#define DIAG_ASSERT_NE(A, B, ...)                                                                                \
	DIAG_ASSERT_OP(A, !=, B, ne, __VA_ARGS__)

#define DIAG_ASSERT_NE(A, B, ...)                                                                                \
	DIAG_ASSERT_OP(A, !=, B, ne, __VA_ARGS__)











/* This prints an "Assertion failed" message and aborts / throws.  */
void LIBDIAG_NORETURN libdiag_assert_fail_comparator(const int64_t a, const int64_t b, const char *assertion_lside, const char *assertion_rside, const char *comparator_name, const char *file, unsigned int line, const char *function, const char *extra_message);

// CONDITION_STR is needed to prevent macros in condition from being expected, which obfuscates
// the logged failure, e.g., "EAGAIN" vs "11".
#define LIBDIAG_ASSERT_COMPARATOR_IMPL(COMPARATOR_FUNCTION, A, B, A_STR, B_STR, ACTION, DETAILS)                                    \
  do {                                                                                             \
	  const int64_t a = (A); \
      const int64_t b = (B);    \
      if (COMPARATOR_FUNCTION(a, b))								\
        ; /* empty */							\
      else								\
	  {                                                                            \
          libdiag_assert_fail_comparator(a, b, A_STR, B_STR, #COMPARATOR_FUNCTION, LIBDIAG_ASSERT_FILE, LIBDIAG_ASSERT_LINE, LIBDIAG_ASSERT_FUNCTION, (DETAILS));	\
		  ACTION;                                                                                      \
      }                                                                                              \
  } while (0)



#define LIBDIAG_ASSERT_COMPARATOR_ORIGINAL(COMPARATOR_OBJ_REF, A, B)   LIBDIAG_ASSERT_COMPARATOR_IMPL(COMPARATOR_OBJ_REF, A, B, #A, #B, LIBDIAG_ASSERT_ACTION, "")
#define LIBDIAG_ASSERT_COMPARATOR_VERBOSE(COMPARATOR_OBJ_REF, A, B, Y) LIBDIAG_ASSERT_COMPARATOR_IMPL(COMPARATOR_OBJ_REF, A, B, #A, #B, LIBDIAG_ASSERT_ACTION, Y)
//#define LIBDIAG_ASSERT_SELECTOR(_1, _2, ASSERT_MACRO, ...) ASSERT_MACRO


#define DIAG_ASSERT_COMPARATOR(COMPARATOR_OBJ_REF, A, B, ...)                                                                                \
  LIBDIAG_EXPAND(LIBDIAG_ASSERT_SELECTOR(__VA_ARGS__, LIBDIAG_ASSERT_COMPARATOR_VERBOSE, LIBDIAG_ASSERT_COMPARATOR_ORIGINAL)(COMPARATOR_OBJ_REF, A, B __VA_OPT__(,) __VA_ARGS__))







int libdiag_string_comparator(const char *a, const char *b);

int libdiag_IEEE754_comparator_absolute_epsilon(const double a, const double b, const double epsilon);
int libdiag_IEEE754_comparator_relative_epsilon(const double a, const double b, const double epsilon);


#define DIAG_ASSERT_STR_EQ(A, B, ...)                                                                                \
  do {                                                                                             \
	  DIAG_ASSERT_COMPARATOR(libdiag_string_comparator, A, B, __VA_ARGS__);                            \
  } while (0)









// these macro names *differ* from their C++ counterparts as C does not support user-defined literals, so we cannot use the `_pct` or `_perunage` suffixes and related tricks to induce polymorphy.

#define DIAG_ASSERT_FLT_EQ_APPROX_PERUNAGE(A, B, REL_EPSILON, ...)                                                                                \
  do {                                                                                             \
	  DIAG_ASSERT_COMPARATOR(libdiag_IEEE754_comparator_relative_epsilon, A, B, REL_EPSILON, __VA_ARGS__);                            \
  } while (0)

#define DIAG_ASSERT_FLT_EQ_APPROX_FIXED(A, B, ABSOLUTE_EPSILON, ...)                                                                                \
  do {                                                                                             \
	  DIAG_ASSERT_COMPARATOR(libdiag_IEEE754_comparator_absolute_epsilon, A, B, ABSOLUTE_EPSILON, __VA_ARGS__);                            \
  } while (0)










// Here's an example why we would (also) need expression-type assertions, next to statement-type assertions::
//
// From the ICU library here's a class constructor snippet:
//
//     constexpr FormattedStringBuilder::Field::Field(uint8_t category, uint8_t field)
//         : bits((
//             U_ASSERT(category <= 0xf),
//             U_ASSERT(field <= 0xf),
//             static_cast<uint8_t>((category << 4) | field)
//         )) {}
//
// Note that U_ASSERT() references the `assert()` macro, which *apparently* is assumed to be an assertion expressiontype!
//















#if defined __cplusplus
}
#endif

#endif // DIAG_ASSERTIONS_MODE

