
#pragma once

#include <assert.h>
#include <stdint.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>

#if defined(_WIN32)
#include <winsock2.h>
#include <windows.h>
#endif


#include <diagnostics/implementation/assertions-common.h>



#define LIBDIAG_ASSERT_VOID_CAST (void)




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
          libdiag_assert_fail(CONDITION_STR, LIBDIAG_ASSERT_C_FILE, LIBDIAG_ASSERT_C_LINE, LIBDIAG_ASSERT_C_FUNCTION, (DETAILS));	\
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

// If ASSERT is called with one argument, the ASSERT_SELECTOR will return
// _ASSERT_ORIGINAL and this will call _ASSERT_ORIGINAL(__VA_ARGS__).
// If ASSERT is called with two arguments, ASSERT_SELECTOR will return
// _ASSERT_VERBOSE, and this will call _ASSERT_VERBOSE,(__VA_ARGS__)
#define DIAG_ASSERT(...)                                                                                \
  LIBDIAG_EXPAND(LIBDIAG_ASSERT_C_SELECTOR(__VA_ARGS__, LIBDIAG_ASSERT_VERBOSE, LIBDIAG_ASSERT_ORIGINAL)(__VA_ARGS__))


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
          libdiag_assert_fail_op(a, b, A_STR, OP_STR, B_STR, #OPERATOR_NAME, LIBDIAG_ASSERT_C_FILE, LIBDIAG_ASSERT_C_LINE, LIBDIAG_ASSERT_C_FUNCTION, (DETAILS));	\
		  ACTION;                                                                                      \
      }                                                                                              \
  } while (0)

#define LIBDIAG_ASSERT_OP_ORIGINAL(A, OP, B, OPERATOR_NAME)   LIBDIAG_ASSERT_OP_IMPL(A, OP, B, #A, #OP, #B, OPERATOR_NAME, LIBDIAG_ASSERT_ACTION, "")
#define LIBDIAG_ASSERT_OP_VERBOSE(A, OP, B, OPERATOR_NAME, Y) LIBDIAG_ASSERT_OP_IMPL(A, OP, B, #A, #OP, #B, OPERATOR_NAME, LIBDIAG_ASSERT_ACTION, Y)
#define LIBDIAG_ASSERT_C_SELECTOR(_1, _2, ASSERT_MACRO, ...)  ASSERT_MACRO


#define DIAG_ASSERT_OP(A, OP, B, OPERATOR_NAME, ...)                                                                                \
  LIBDIAG_EXPAND(LIBDIAG_ASSERT_C_SELECTOR(__VA_ARGS__, LIBDIAG_ASSERT_OP_VERBOSE, LIBDIAG_ASSERT_OP_ORIGINAL)(A, OP, B, OPERATOR_NAME __VA_OPT__(,) __VA_ARGS__))







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
#define LIBDIAG_ASSERT_COMPARATOR_IMPL(A, B, COMPARATOR_OBJ_REF, A_STR, B_STR, ACTION, DETAILS, ...)                                    \
  do {                                                                                             \
      /* ensure A and B are only evaluated *once*, just like in our C++ implementation! */ \
      if ((COMPARATOR_OBJ_REF)->matches(COMPARATOR_OBJ_REF, A, B))								\
        ; /* empty */							\
      else								\
	  {                                                                            \
		static const struct ReportFailureBaseInfo info = { \
			.assertion_lside = A_STR, \
			.assertion_operand = nullptr, /* unused */ \
			.assertion_rside = B_STR, \
			.operand_name = #COMPARATOR_OBJ_REF, \
			.file = LIBDIAG_ASSERT_C_FILE, \
			.function = LIBDIAG_ASSERT_C_FUNCTION, \
			.line = LIBDIAG_ASSERT_C_LINE, \
		}; \
		(COMPARATOR_OBJ_REF)->report_failure(COMPARATOR_OBJ_REF, &info, (DETAILS) __VA_OPT__(,) __VA_ARGS__);	\
 	    ACTION;                                                                                      \
      }                                                                                              \
  } while (0)



#define LIBDIAG_ASSERT_COMPARATOR_ORIGINAL(A, B, COMPARATOR_OBJ_REF, ...)   LIBDIAG_ASSERT_COMPARATOR_IMPL(A, B, COMPARATOR_OBJ_REF, #A, #B, LIBDIAG_ASSERT_ACTION, "", __VA_ARGS__)
#define LIBDIAG_ASSERT_COMPARATOR_VERBOSE(A, B, COMPARATOR_OBJ_REF, Y, ...) LIBDIAG_ASSERT_COMPARATOR_IMPL(A, B, COMPARATOR_OBJ_REF, #A, #B, LIBDIAG_ASSERT_ACTION, Y, __VA_ARGS__)
//#define LIBDIAG_ASSERT_C_SELECTOR(_1, _2, ASSERT_MACRO, ...) ASSERT_MACRO


#define DIAG_ASSERT_COMPARATOR(A, B, COMPARATOR_OBJ_REF, ...)                                                                                \
  LIBDIAG_EXPAND(LIBDIAG_ASSERT_C_SELECTOR(foobar_1 __VA_OPT__(,) foobar_2, LIBDIAG_ASSERT_COMPARATOR_VERBOSE, LIBDIAG_ASSERT_COMPARATOR_ORIGINAL)(A, B, COMPARATOR_OBJ_REF, __VA_ARGS__))







struct StringComparatorSpec;

typedef bool StringComparator_matches_f(/* const */ struct StringComparatorSpec* spec, const char *a, const char *b);

struct ReportFailureBaseInfo {
	const char *assertion_lside;
	const char *assertion_operand;
	const char *assertion_rside;
	const char *operand_name;

	const char *file;
	const char *function;
	unsigned int line;
};

typedef void /* LIBDIAG_NORETURN */ StringComparator_assert_fail_reporter_f(/* const */ struct StringComparatorSpec* spec, const struct ReportFailureBaseInfo* assertion_info, const char *extra_message, ...);


struct StringComparatorSpec {
	StringComparator_matches_f* matches;
	StringComparator_assert_fail_reporter_f *report_failure;

	// stored values of the comparisands: we store these in this struct temporarily to allow a macro implementation where
	// both A and B are only evaluated exactly *once* while also ensuring we don't need the type of either A or B to be known
	// at macro invocation time: the language does not sport language features such as C++ `auto` and `decltype()`, regrettably.
	const char *operand_A_value;
	const char *operand_B_value;
};

bool StringComparator_matches(struct StringComparatorSpec* spec, const char *a, const char *b) {
	if (a == b)
		return true;
	const int diff = strcmp(a, b);
	if (diff == 0)
		return true;

	// Note: we only need to store the operand values for A and B *iff* we are going to invoke the assertion failure report function.
	spec->operand_A_value = a;
	spec->operand_B_value = b;

	return false;
}

#ifndef nullptr
#define nullptr  NULL
#endif

// LAMENT: I would've preferred to use named parameters here: https://pdimov.github.io/blog/2020/09/07/named-parameters-in-c20/
struct StringComparatorSpec mk_StringComparator(void) {
	struct StringComparatorSpec spec = {
		.matches = StringComparator_matches,

		.operand_A_value = nullptr, // default value, will be overwritten by the each failing call to `matches()`
		.operand_B_value = nullptr, // default value, will be overwritten by the each failing call to `matches()`
	};

	return spec;
}

struct StringComparatorSpec* get_common_StringComparator(void) {
	static struct StringComparatorSpec spec = {.matches = nullptr};
	if (!spec.matches)
		spec = mk_StringComparator();
	return &spec;
}


#define DIAG_ASSERT_STR_EQ(A, B, ...)                                                                                \
  do {                                                                                             \
	  DIAG_ASSERT_COMPARATOR(A, B, get_common_StringComparator(), __VA_ARGS__);                            \
  } while (0)









// these macro names *differ* from their C++ counterparts as C does not support user-defined literals, so we cannot use the `_pct` or `_perunage` suffixes and related tricks to induce polymorphy.

#define DIAG_ASSERT_FLT_EQ_APPROX_PERUNAGE(A, B, REL_EPSILON, ...)                                                                                \
  do {                                                                                             \
    struct IEEE754ComparatorSpec spec = mk_IEEE754Comparator(REL_EPSILON, -1.0); \
    DIAG_ASSERT_COMPARATOR(A, B, &spec, __VA_ARGS__);                            \
  } while (0)

#define DIAG_ASSERT_FLT_EQ_APPROX_FIXED(A, B, ABSOLUTE_EPSILON, ...)                                                                                \
  do {                                                                                             \
    struct IEEE754ComparatorSpec spec = mk_IEEE754Comparator(-1.0, ABSOLUTE_EPSILON); \
    DIAG_ASSERT_COMPARATOR(A, B, &spec, __VA_ARGS__);                            \
  } while (0)



#define DIAG_ASSERT_FLT_EQ_APPROX(A, B, ...)                                                                                \
  do {                                                                                             \
	  DIAG_ASSERT_COMPARATOR(A, B, get_common_IEEE754Comparator(), __VA_ARGS__);                            \
  } while (0)

#define DIAG_ASSERT_FLT_EQ_APPROX_EX(A, B, IEEE754COMPARATOR_OBJ_REF, ...)                                                                                \
  do {                                                                                             \
	  DIAG_ASSERT_COMPARATOR(A, B, IEEE754COMPARATOR_OBJ_REF, __VA_ARGS__);                            \
  } while (0)





struct IEEE754ComparatorSpec;

typedef bool IEEE754Comparator_matches_f(/* const */ struct IEEE754ComparatorSpec* spec, const double a, const double b);

typedef void /* LIBDIAG_NORETURN */ IEEE754Comparator_assert_fail_reporter_f(/* const */ struct IEEE754ComparatorSpec* spec, const struct ReportFailureBaseInfo* assertion_info, const char *extra_message, ...);

struct IEEE754ComparatorParameters {
	// -1.0	means "default value"; or rather: *any* negative value signals you want to apply the "default value".
	// 
	// Note: 0.0 is a valid epsilon/tolerance value, so we need to use -1.0 as a marker for "default value".
	double absolute_epsilon;
	double relative_epsilon;
};

struct IEEE754ComparatorSpec {
	struct IEEE754ComparatorParameters settings;

	IEEE754Comparator_matches_f* matches;
	IEEE754Comparator_assert_fail_reporter_f* report_failure;

	// stored values of the comparisands: we store these in this struct temporarily to allow a macro implementation where
	// both A and B are only evaluated exactly *once* while also ensuring we don't need the type of either A or B to be known
	// at macro invocation time: the language does not sport language features such as C++ `auto` and `decltype()`, regrettably.
	double operand_A_value;
	double operand_B_value;
};

bool IEEE754Comparator_matches(struct IEEE754ComparatorSpec* spec, const double a, const double b) {
	if (a == b)
		return true;
	const double diff = fabs(a - b);
	if (diff < spec->settings.absolute_epsilon)
		return true;
	const double ah = fabs(a);
	const double bh = fabs(b);
	// make sure we don't collide with the MSVC `max()` **macro**!
	const double largest_value = fmax(ah, bh);
	const double part = largest_value + spec->settings.relative_epsilon;

	if (diff <= part)
		return true;

	// Note: we only need to store the operand values for A and B *iff* we are going to invoke the assertion failure report function.
	spec->operand_A_value = a;
	spec->operand_B_value = b;

	return false;
}

// LAMENT: I would've preferred to use named parameters here: https://pdimov.github.io/blog/2020/09/07/named-parameters-in-c20/
struct IEEE754ComparatorSpec mk_IEEE754Comparator(double _absolute_epsilon, double _relative_epsilon) {
	struct IEEE754ComparatorSpec spec = {
		.settings = {
			.absolute_epsilon = _absolute_epsilon < 0.0 ? 1e-35 : _absolute_epsilon,
			.relative_epsilon = _relative_epsilon < 0.0 ? 1e-7 : _relative_epsilon,
		},
		.matches = IEEE754Comparator_matches,

		.operand_A_value = 0.0, // default value, will be overwritten by the each failing call to `matches()`
		.operand_B_value = 0.0, // default value, will be overwritten by the each failing call to `matches()`
	};

	return spec;
}

struct IEEE754ComparatorSpec mk_IEEE754Comparator_alt(const struct IEEE754ComparatorParameters* _settings) {
	struct IEEE754ComparatorSpec spec = {
		.settings = *_settings,
		.matches = IEEE754Comparator_matches,

		.operand_A_value = 0.0, // default value, will be overwritten by the each failing call to `matches()`
		.operand_B_value = 0.0, // default value, will be overwritten by the each failing call to `matches()`
	};

	return spec;
}

struct IEEE754ComparatorSpec* get_common_IEEE754Comparator(void) {
	static struct IEEE754ComparatorSpec spec = {.matches = nullptr};
	if (!spec.matches)
		spec = mk_IEEE754Comparator(-1.0, -1.0);
	return &spec;
}








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

