
#pragma once

#if !defined __cplusplus
#error "This header is only for C++ code, not for C code."
#endif

#include <cassert>
#include <format>
#include <algorithm>
#include <type_traits>
#include <cstddef>
#include <numbers>
#include <string>

#if defined(_WIN32)
#include <winsock2.h>
#include <windows.h>
#endif


#include <diagnostics/implementation/assertions-common.h>



# define LIBDIAG_ASSERT_VOID_CAST static_cast<void>


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

// Version 2.4 and later of GCC define a magical variable `__PRETTY_FUNCTION__'
// which contains the name of the function currently being defined.
// This is broken in G++ before version 2.6.
// C9x has a similar variable called __func__, but prefer the GCC one since
// it demangles C++ function names.

#if defined(__GNUC__) && (__GNUC__ >= 3)
#  define LIBDIAG_ASSERT_FUNCTION	__PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#  define LIBDIAG_ASSERT_FUNCTION	__FUNCSIG__
#else
#  define LIBDIAG_ASSERT_FUNCTION	__func__
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



namespace libdiag {

	/* This prints an "Assertion failed" message and aborts / throws.  */
	void LIBDIAG_NORETURN assert_fail(const char *assertion, const char *file, unsigned int line, const char *function, const char *extra_message);
	void LIBDIAG_NORETURN assert_fail(const char *assertion, const char *file, unsigned int line, const char *function, const std::string &extra_message);

	/* Likewise, but prints the error text for ERRNUM.  */
	void LIBDIAG_NORETURN perror_fail(int errnum, const char *assertion, const char *file, unsigned int line, const char *function, const char *extra_message);
	void LIBDIAG_NORETURN perror_fail(int errnum, const char *assertion, const char *file, unsigned int line, const char *function, const std::string &extra_message);

#if defined(_WIN32)

	/* Likewise, but prints the error text for ERRNUM.  */
	void LIBDIAG_NORETURN Win32error_fail(HRESULT errnum, const char *assertion, const char *file, unsigned int line, const char *function, const char *extra_message);
	void LIBDIAG_NORETURN Win32error_fail(HRESULT errnum, const char *assertion, const char *file, unsigned int line, const char *function, const std::string &extra_message);

#endif

} // namespace libdiag




#if DIAG_ASSERTIONS_MODE == 0

#define DIAG_ASSERT(expr)		(LIBDIAG_ASSERT_VOID_CAST (0))

#else // DIAG_ASSERTIONS_MODE








// assertion macro mapping for 1 and 2 arguments: stolen from Intel Envoy (https://github.com/envoyproxy/envoy)

// CONDITION_STR is needed to prevent macros in condition from being expected, which obfuscates
// the logged failure, e.g., "EAGAIN" vs "11".
#define LIBDIAG_ASSERT_IMPL(CONDITION, CONDITION_STR, ACTION, DETAILS)                                    \
  do {                                                                                             \
      if (static_cast<bool>(CONDITION))								\
        ; /* empty */							\
      else								\
	  {                                                                            \
          libdiag::assert_fail(CONDITION_STR, LIBDIAG_ASSERT_FILE, LIBDIAG_ASSERT_LINE, LIBDIAG_ASSERT_FUNCTION, (DETAILS));	\
		  ACTION;                                                                                      \
      }                                                                                              \
  } while (false)

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
    } while (false)






// This non-implementation ensures that its argument is a valid expression that can be statically
// casted to a bool, but the expression is never evaluated and will be compiled away.
#define LIBDIAG_NULL_ASSERT_IMPL(X, ...)                                                           \
  do {                                                                                             \
    constexpr bool __assert_dummy_variable = false && static_cast<bool>(X);                        \
    (void)__assert_dummy_variable;                                                                 \
  } while (false)







/* This prints an "Assertion failed" message and aborts / throws.  */
template <typename TA, typename TB>
void LIBDIAG_NORETURN libdiag_assert_fail_op(const TA& a, const TB& b, const char *assertion_lside, const char *assertion_operand, const char *assertion_rside, const char *operand_name, const char *file, unsigned int line, const char *function, const char *extra_message);


// CONDITION_STR is needed to prevent macros in condition from being expected, which obfuscates
// the logged failure, e.g., "EAGAIN" vs "11".
#define LIBDIAG_ASSERT_OP_IMPL(A, OP, B, A_STR, OP_STR, B_STR, OPERATOR_NAME, ACTION, DETAILS)                                    \
  do {                                                                                             \
	  using A_TYPE = decltype(A); \
	  using B_TYPE = decltype(B); \
      static_assert(std::is_object<A_TYPE>::value, "A must be an object instance"); \
	  static_assert(std::is_object<B_TYPE>::value, "B must be an object instance"); \
	  static_assert(std::is_layout_compatible<A_TYPE, B_TYPE>::value, "A and B must be layout compatible"); \
	  static_assert(std::is_same<decltype(A OP B), bool>::value, "A OP B must return a bool"); \
	  auto a{(A)};    \
	  auto b{(B)};    \
      if (static_cast<bool>(a OP b))								\
        ; /* empty */							\
      else								\
	  {                                                                            \
          libdiag_assert_fail_op<decltype(A), decltype(B)>(a, b, A_STR, OP_STR, B_STR, #OPERATOR_NAME, LIBDIAG_ASSERT_FILE, LIBDIAG_ASSERT_LINE, LIBDIAG_ASSERT_FUNCTION, (DETAILS));	\
		  ACTION;                                                                                      \
      }                                                                                              \
  } while (false)


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











namespace libdiag {
	template <typename T1, typename T2>
	class ComparatorBase {
	public:
		~ComparatorBase() = default;

		bool matches(const T1& a, const T2& b) const;
		const char *nameof() const;
	};

	/* This prints an "Assertion failed" message and aborts / throws.  */
	template <typename TA, typename TB>
	void LIBDIAG_NORETURN assert_fail_comparator(const TA& a, const TB& b, const char *assertion_lside, const char *assertion_rside, const char *comparator_name, const char *file, unsigned int line, const char *function, const char *extra_message);
	template <typename TA, typename TB>
	void LIBDIAG_NORETURN assert_fail_comparator(const TA& a, const TB& b, const char *assertion_lside, const char *assertion_rside, const char *comparator_name, const char *file, unsigned int line, const char *function, const std::string &extra_message);

}

// CONDITION_STR is needed to prevent macros in condition from being expected, which obfuscates
// the logged failure, e.g., "EAGAIN" vs "11".
#define LIBDIAG_ASSERT_COMPARATOR_IMPL(COMPARATOR_OBJ_REF, A, B, A_STR, B_STR, ACTION, DETAILS)                                    \
  do {                                                                                             \
	  const auto& a{(A)}; \
      const auto& b{(B)};    \
	  using COMPARATOR_TYPE = decltype(COMPARATOR_OBJ_REF); \
	  static_assert(std::is_object<COMPARATOR_TYPE>::value, "COMPARATOR_OBJ_REF must be an object instance"); \
	  static_assert(std::is_class<COMPARATOR_TYPE>::value, "COMPARATOR_OBJ_REF must be of a comparator class type"); \
	  /* static_assert(std::is_layout_compatible<libdiag::ComparatorBase<decltype(A), decltype(B)>, COMPARATOR_TYPE>::value, "COMPARATOR_OBJ_REF must be a valid comparator object instance"); */ \
	  /* static_assert(std::is_base_of<libdiag::ComparatorBase<decltype(A), decltype(B)>, COMPARATOR_TYPE>::value, "COMPARATOR_OBJ_REF must be an instance of a libdiag::ComparatorBase<> or have libdiag::ComparatorBase<> as one of its base classes"); */ \
	  static_assert(std::is_same<decltype(COMPARATOR_OBJ_REF.matches(a, b)), bool>::value, "COMPARATOR_OBJ_REF.matches must return a bool"); \
	  static_assert(std::is_same<decltype(COMPARATOR_OBJ_REF.nameof()), const char *>::value, "COMPARATOR_OBJ_REF.nameof must return a const char*"); \
	  auto& COMPOBJ = (COMPARATOR_OBJ_REF);				\
      if (COMPOBJ.matches(a, b))								\
        ; /* empty */							\
      else								\
	  {                                                                            \
          libdiag::assert_fail_comparator<decltype(A), decltype(B)>(a, b, A_STR, B_STR, COMPOBJ.nameof(), LIBDIAG_ASSERT_FILE, LIBDIAG_ASSERT_LINE, LIBDIAG_ASSERT_FUNCTION, (DETAILS));	\
		  ACTION;                                                                                      \
      }                                                                                              \
  } while (false)


#define LIBDIAG_ASSERT_COMPARATOR_ORIGINAL(COMPARATOR_OBJ_REF, A, B)   LIBDIAG_ASSERT_COMPARATOR_IMPL(COMPARATOR_OBJ_REF, A, B, #A, #B, LIBDIAG_ASSERT_ACTION, "")
#define LIBDIAG_ASSERT_COMPARATOR_VERBOSE(COMPARATOR_OBJ_REF, A, B, Y) LIBDIAG_ASSERT_COMPARATOR_IMPL(COMPARATOR_OBJ_REF, A, B, #A, #B, LIBDIAG_ASSERT_ACTION, Y)
//#define LIBDIAG_ASSERT_SELECTOR(_1, _2, ASSERT_MACRO, ...) ASSERT_MACRO


#define DIAG_ASSERT_COMPARATOR(COMPARATOR_OBJ_REF, A, B, ...)                                                                                \
  LIBDIAG_EXPAND(LIBDIAG_ASSERT_SELECTOR(__VA_ARGS__, LIBDIAG_ASSERT_COMPARATOR_VERBOSE, LIBDIAG_ASSERT_COMPARATOR_ORIGINAL)(COMPARATOR_OBJ_REF, A, B __VA_OPT__(,) __VA_ARGS__))







namespace libdiag {

	class StringComparator: public ComparatorBase<std::string, std::string> {
	public:
		bool matches(const std::string& a, const std::string& b) const {
			return (strcmp(a.c_str(), b.c_str()) == 0);
		}
		const char *nameof() const {
			return "StringComparator";
		}
	};

}

namespace libdiag {

	// https://en.cppreference.com/w/cpp/language/user_literal.html

#if !defined(__cpp_user_defined_literals)
#error "libdiagnostics requires a compiler which supports C++ user-defined literal suffixes: https://en.cppreference.com/w/cpp/language/user_literal.html"
#endif

	class perunage_t {
		long double mP;
	public:
		perunage_t() = delete;
		perunage_t(long double pct)
			: mP(std::abs(pct))
		{
		}
		~perunage_t() = default;

		static perunage_t minimum_4_float(void) {
			return perunage_t{FLT_EPSILON}; // ≅ 1e-07
		}
		static perunage_t minimum_4_double(void) {
			return perunage_t{DBL_EPSILON}; // ≅ 1e-16
		}
		static perunage_t default_4_float(void) {
			return perunage_t{1e-4};
		}
		static perunage_t default_4_double(void) {
			return perunage_t{1e-9};
		}

		constexpr double value(void) {
			return mP;
		}
	};

}



// used as conversion from percentage (input param) to per-unage (returned output)
libdiag::perunage_t operator""_pct(long double pct)
{
	long double pu = pct * 0.01;
	return pu;
}

libdiag::perunage_t operator""_perunage(long double perunage)
{
	return perunage;
}

libdiag::perunage_t operator""_promille(long double ppm)
{
	long double pu = ppm * 0.001;
	return pu;
}



namespace libdiag {

	class IEEE754Comparator: public ComparatorBase<std::string, std::string> {
	protected:
		double mAbsoluteEpsilon;
		double mRelativeEpsilon;
		std::string mName;

	public:
		struct parameters_t {
			// -1.0	means "default value"; or rather: *any* negative value signals you want to apply the "default value".
			// 
			// Note: 0.0 is a valid epsilon/tolerance value, so we need to use -1.0 as a marker for "default value".
			double absolute_epsilon{-1.0};	
			double relative_epsilon{-1.0};
		};

		// LAMENT: I would've preferred to use named parameters here: https://pdimov.github.io/blog/2020/09/07/named-parameters-in-c20/
		explicit IEEE754Comparator(double absolute_epsilon = -1.0, double relative_epsilon = -1.0)
			: mAbsoluteEpsilon(absolute_epsilon < 0.0 ? 1e-35 : absolute_epsilon)
			, mRelativeEpsilon(relative_epsilon < 0.0 ? 1e-7 : relative_epsilon)
			, mName(std::format("IEEE754Comparator<Rel.Eps:{:g}; Abs.Eps:{:g}>", relative_epsilon, absolute_epsilon))
		{}
		// The way to pass in only a modified *relative epsilon* value, while keeping the default absolute epsilon value.
		explicit IEEE754Comparator(perunage_t relative_epsilon, double absolute_epsilon = -1)
			: IEEE754Comparator(relative_epsilon.value(), absolute_epsilon)
		{
		}
		explicit IEEE754Comparator(parameters_t settings)
			: IEEE754Comparator(settings.relative_epsilon, settings.absolute_epsilon)
		{
		}

		bool matches(const double a, const double b) const {
			if (a == b)
				return true;
			const double diff = std::abs(a - b);
			if (diff < mAbsoluteEpsilon)
				return true;
			const double ah = std::abs(a);
			const double bh = std::abs(b);
			// make sure we don't collide with the MSVC `max()` **macro**!
			const double largest_value = (std::max)(ah, bh);
			const double part = largest_value + mRelativeEpsilon;
			return (diff <= part);
		}

		const char *nameof() const {
			return mName.c_str();
		}

		parameters_t parameters() const {
			return {.absolute_epsilon = mAbsoluteEpsilon, .relative_epsilon = mRelativeEpsilon};
		}
	};
}


#define DIAG_ASSERT_STR_EQ(A, B, ...)                                                                                \
  do {                                                                                             \
	  libdiag::StringComparator cmp{};													\
	  DIAG_ASSERT_COMPARATOR(cmp, A, B, __VA_ARGS__);                            \
  } while (false)










#define DIAG_ASSERT_FLT_EQ_APPROX(A, B, REL_EPSILON, ...)                                                                                \
  do {                                                                                             \
	  /* libdiag::IEEE754Comparator flt_cmp{{.relative_epsilon = REL_EPSILON }}; */													\
	  libdiag::IEEE754Comparator flt_cmp{REL_EPSILON};													\
	  DIAG_ASSERT_COMPARATOR(flt_cmp, A, B, __VA_ARGS__);                            \
  } while (false)












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















#endif // DIAG_ASSERTIONS_MODE
























#if 0



















// GCC does not allow ##__VA_ARGS__ unless GNU extensions are enabled (--std=gnu++NN instead of
// --std=c++NN) and I think we don't want that, so we'll have a fallback path for GCC. Obviously
// this will not actually succeed at getting the desired info into registers before crashing, but
// it's just a fallback anyway.
//
// FIXME: When we enable C++20, we should replace ##__VA_ARGS__ with format __VA_OPT__(,) __VA_ARGS__
// so that we can remove this fallback.














// Since C+11 When # appears before __VA_ARGS__, the entire expanded
// __VA_ARGS__ is enclosed in quotes



















// Provide static_assert as a macro before C23.
#if !defined(__cplusplus) && __STDC_VERSION__ < 202311L

#undef static_assert  // Undefine the macro in case <assert.h> is included.

// Define static_assert to support an optional message argument, as in C23.
#define static_assert(expression, ...) \
  _Static_assert(expression, "(" #expression ") " __VA_ARGS__)

// _Static_assert was not added until C11, but modern compilers support it for
// all C standards. GCC has supported it since 4.6, for example.

#endif  // !defined(__cplusplus) && __STDC_VERSION__ < 202311L






















































#if XXXXXXXXXXXXXXXXXXXXXXXXX
#  define assert(expr)							\
     (static_cast <bool> (expr)						\
      ? void (0)							\
      : __assert_fail (#expr, __ASSERT_FILE, __ASSERT_LINE,             \
                       __ASSERT_FUNCTION))
# elif !defined __GNUC__ || defined __STRICT_ANSI__
#  define assert(expr)							\
    ((expr)								\
     ? __ASSERT_VOID_CAST (0)						\
     : __assert_fail (#expr, __FILE__, __LINE__, __ASSERT_FUNCTION))
# else
   /* The first occurrence of EXPR is not evaluated due to the sizeof,
	  but will trigger any pedantic warnings masked by the __extension__
	  for the second occurrence.  The ternary operator is required to
	  support function pointers and bit fields in this context, and to
	  suppress the evaluation of variable length arrays.  */
#  define assert(expr)							\
  ((void) sizeof ((expr) ? 1 : 0), __extension__ ({			\
      if (expr)								\
        ; /* empty */							\
      else								\
        __assert_fail (#expr, __FILE__, __LINE__, __ASSERT_FUNCTION);	\
    }))
# endif

# ifdef	__USE_GNU
#  define assert_perror(errnum)						\
  (!(errnum)								\
   ? __ASSERT_VOID_CAST (0)						\
   : __assert_perror_fail ((errnum), __FILE__, __LINE__, __ASSERT_FUNCTION))
# endif


#if (defined __USE_ISOC11			\
     && (!defined __STDC_VERSION__		\
	 || __STDC_VERSION__ <= 201710L		\
	 || !__GNUC_PREREQ(13, 0))		\
     && !defined __cplusplus)
# undef static_assert
# define static_assert _Static_assert
#endif







namespace raul
{

	/**
	 * Exception with tracing
	 */
	class Exception: public std::exception
	{
		std::string message;
		std::optional<std::string> typeName;
		std::optional<std::string> objName;
		std::optional<std::string> funcName;
		std::optional<std::string> fileName;
		std::optional<size_t> lineNumber;

		std::string errorText;
		std::exception_ptr innerException;

		[[nodiscard]] std::string getMessage(size_t level = 0) const;

	public:
		explicit Exception(std::string message)
			: message{std::move(message)}
			, typeName{std::nullopt}
			, objName{std::nullopt}
			, funcName{std::nullopt}
			, fileName{std::nullopt}
			, lineNumber{std::nullopt}
		{
			errorText = getMessage();
		}

		Exception setType(std::string name)
		{
			typeName = std::move(name);
			errorText = getMessage();
			return *this;
		}

		Exception setObject(std::string name)
		{
			objName = std::move(name);
			errorText = getMessage();
			return *this;
		}

		Exception setFunction(std::string name)
		{
			funcName = std::move(name);
			errorText = getMessage();
			return *this;
		}

		Exception setPosition(std::string name, size_t line)
		{
			fileName = std::move(name);
			lineNumber = line;
			errorText = getMessage();
			return *this;
		}

		Exception setInnerException(std::exception_ptr exception)
		{
			if (exception)
			{
				innerException = std::move(exception);
				errorText = getMessage();
			}
			return *this;
		}

		[[nodiscard]] const char* what() const noexcept override;
		~Exception() {}
	};

#define BASE_TYPE_NAME(TYPE) (std::is_same_v<TYPE, dtype> ? "dtype" : std::is_same_v<TYPE, half> ? "half" : "T")

#define THROW(TYPE, OBJECT, MESSAGE) throw raul::Exception(MESSAGE).setType(TYPE).setObject(OBJECT).setFunction(__func__).setPosition(__FILE__, __LINE__).setInnerException(std::current_exception());
#define THROW_NONAME(TYPE, MESSAGE) throw raul::Exception(MESSAGE).setType(TYPE).setFunction(__func__).setPosition(__FILE__, __LINE__).setInnerException(std::current_exception());

} // namespace raul



#define BSON_ASSERT(test)                                                                                 \
   do {                                                                                                   \
      if (!(BSON_LIKELY (test))) {                                                                        \
         fprintf (stderr, "%s:%d %s(): precondition failed: %s\n", __FILE__, __LINE__, BSON_FUNC, #test); \
         abort ();                                                                                        \
      }                                                                                                   \
   } while (0)

/**
 * @brief Assert the expression `Assertion`, and evaluates to `Value` on
 * success.
 */
#define BSON_ASSERT_INLINE(Assertion, Value)                                                                         \
   ((void) ((Assertion)                                                                                              \
               ? (0)                                                                                                 \
               : ((fprintf (stderr, "%s:%d %s(): Assertion '%s' failed", __FILE__, __LINE__, BSON_FUNC, #Assertion), \
                   abort ()),                                                                                        \
                  0)),                                                                                               \
    Value)

 /**
  * @brief Assert that the given pointer is non-NULL, while also evaluating to
  * that pointer.
  *
  * Can be used to inline assertions with a pointer dereference:
  *
  * ```
  * foo* f = get_foo();
  * bar* b = BSON_ASSERT_PTR_INLINE(f)->bar_value;
  * ```
  */
#define BSON_ASSERT_PTR_INLINE(Pointer) BSON_ASSERT_INLINE ((Pointer) != NULL, (Pointer))

  /* Used for asserting parameters to provide a more precise error message */
#define BSON_ASSERT_PARAM(param)                                                                     \
   do {                                                                                              \
      if ((BSON_UNLIKELY (param == NULL))) {                                                         \
         fprintf (stderr, "The parameter: %s, in function %s, cannot be NULL\n", #param, BSON_FUNC); \
         abort ();                                                                                   \
      }                                                                                              \
   } while (0)



















#if defined(__OPTIMIZE__) && hb_has_builtin(__builtin_expect)
#define likely(expr) __builtin_expect (bool(expr), 1)
#define unlikely(expr) __builtin_expect (bool(expr), 0)
#else
#define likely(expr) (expr)
#define unlikely(expr) (expr)
#endif

#if !defined(__GNUC__) && !defined(__clang__)
#undef __attribute__
#define __attribute__(x)
#endif

#if defined(__GNUC__) && (__GNUC__ >= 3)
#define HB_PRINTF_FUNC(format_idx, arg_idx) __attribute__((__format__ (__printf__, format_idx, arg_idx)))
#else
#define HB_PRINTF_FUNC(format_idx, arg_idx)
#endif
#if defined(__GNUC__) && (__GNUC__ >= 4) || (__clang__)
#define HB_UNUSED	__attribute__((unused))
#elif defined(_MSC_VER) /* https://github.com/harfbuzz/harfbuzz/issues/635 */
#define HB_UNUSED __pragma(warning(suppress: 4100 4101))
#else
#define HB_UNUSED
#endif















==========================================================================================================================
FILE: ./__misc/envoy___intel/source/common/common/assert.h
--------------------------------------------------------------------------------------------------------------------------

#pragma once

#include <functional>

#include "source/common/common/logger.h"

#include "absl/debugging/stacktrace.h"
#include "absl/debugging/symbolize.h"

namespace Envoy {
	namespace Assert {

		class ActionRegistration {
		public:
			virtual ~ActionRegistration() = default;
		};
		using ActionRegistrationPtr = std::unique_ptr<ActionRegistration>;

		/*
		 * EnvoyBugStackTrace captures and writes the stack trace to Envoy Bug
		 * to assist with getting additional context for reports.
		 */
		class EnvoyBugStackTrace: private Logger::Loggable<Logger::Id::envoy_bug> {
		public:
			EnvoyBugStackTrace() = default;
			/*
			 * Capture the stack trace.
			 * Skip count is one as to skip the last call which is capture().
			 */
			void capture() {
				stack_depth_ = absl::GetStackTrace(stack_trace_, kMaxStackDepth, /* skip_count = */ 1);
			}

			/*
			 * Logs each row of the captured stack into the envoy_bug log.
			 */
			void logStackTrace() {
				ENVOY_LOG(error, "stacktrace for envoy bug");
				char out[1024];
				for (int i = 0; i < stack_depth_; ++i) {
					const bool success = absl::Symbolize(stack_trace_[i], out, sizeof(out));
					if (success) {
						ENVOY_LOG(error, "#{} {} [{}]", i, out, stack_trace_[i]);
					} else {
						ENVOY_LOG(error, "#{} {} [{}]", i, "UNKNOWN", stack_trace_[i]);
					}
				}
			}

		private:
			static const int kMaxStackDepth = 16;
			void* stack_trace_[kMaxStackDepth];
			int stack_depth_{0};
		};

		/**
		 * Sets an action to be invoked when a debug assertion failure is detected
		 * in a release build. This action will be invoked each time an assertion
		 * failure is detected.
		 *
		 * This function is not thread-safe; concurrent calls to set the action are not allowed.
		 *
		 * The action may be invoked concurrently if two ASSERTS in different threads fail at the
		 * same time, so the action must be thread-safe.
		 *
		 * This has no effect in debug builds (assertion failure aborts the process)
		 * or in release builds without ENVOY_LOG_DEBUG_ASSERT_IN_RELEASE defined (assertion
		 * tests are compiled out).
		 *
		 * @param action The action to take when an assertion fails.
		 * @return A registration object. The registration is removed when the object is destructed.
		 */
		ActionRegistrationPtr
			addDebugAssertionFailureRecordAction(const std::function<void(const char* location)>& action);

		/**
		 * Sets an action to be invoked when an ENVOY_BUG failure is detected in a release build. This
		 * action will be invoked each time an ENVOY_BUG failure is detected.
		 *
		 * This function is not thread-safe; concurrent calls to set the action are not allowed.
		 *
		 * The action may be invoked concurrently if two ENVOY_BUGs in different threads fail at the
		 * same time, so the action must be thread-safe.
		 *
		 * This has no effect in debug builds (envoy bug failure aborts the process).
		 *
		 * @param action The action to take when an envoy bug fails.
		 * @return A registration object. The registration is removed when the object is destructed.
		 */
		ActionRegistrationPtr
			addEnvoyBugFailureRecordAction(const std::function<void(const char* location)>& action);

		/**
		 * Invokes the action set by setDebugAssertionFailureRecordAction, or does nothing if
		 * no action has been set.
		 *
		 * @param location Unique identifier for the ASSERT, currently source file and line.
		 *
		 * This should only be called by ASSERT macros in this file.
		 */
		void invokeDebugAssertionFailureRecordActionForAssertMacroUseOnly(const char* location);

		/**
		 * Invokes the action set by setEnvoyBugFailureRecordAction, or does nothing if
		 * no action has been set.
		 *
		 * @param location Unique identifier for the ENVOY_BUG, currently source file and line.
		 *
		 * This should only be called by ENVOY_BUG macros in this file.
		 */
		void invokeEnvoyBugFailureRecordActionForEnvoyBugMacroUseOnly(const char* location);

		/**
		 * Increments power of two counter for EnvoyBugRegistrationImpl.
		 *
		 * @param bug_name Unique identifier for the ENVOY_BUG, currently source file and line.
		 * @return True if the hit count is equal to a power of two after increment.
		 *
		 * This should only be called by ENVOY_BUG macros in this file.
		 */
		bool shouldLogAndInvokeEnvoyBugForEnvoyBugMacroUseOnly(absl::string_view bug_name);

		/**
		 * Resets all counters for EnvoyBugRegistrationImpl between tests.
		 *
		 */
		void resetEnvoyBugCountersForTest();

		// CONDITION_STR is needed to prevent macros in condition from being expected, which obfuscates
		// the logged failure, e.g., "EAGAIN" vs "11".
#define _ASSERT_IMPL(CONDITION, CONDITION_STR, ACTION, DETAILS)                                    \
  do {                                                                                             \
    if (!(CONDITION)) {                                                                            \
      const std::string& details = (DETAILS);                                                      \
      ENVOY_LOG_TO_LOGGER(Envoy::Logger::Registry::getLog(Envoy::Logger::Id::assert), critical,    \
                          "assert failure: {}.{}{}", CONDITION_STR,                                \
                          details.empty() ? "" : " Details: ", details);                           \
      ACTION;                                                                                      \
    }                                                                                              \
  } while (false)

// This non-implementation ensures that its argument is a valid expression that can be statically
// casted to a bool, but the expression is never evaluated and will be compiled away.
#define _NULL_ASSERT_IMPL(X, ...)                                                                  \
  do {                                                                                             \
    constexpr bool __assert_dummy_variable = false && static_cast<bool>(X);                        \
    (void)__assert_dummy_variable;                                                                 \
  } while (false)

/**
 * assert macro that uses our builtin logging which gives us thread ID and can log to various
 * sinks.
 *
 * The old style release assert was of the form RELEASE_ASSERT(foo == bar);
 * where it would log stack traces and the failed conditional and crash if the
 * condition is not met. The are many legacy RELEASE_ASSERTS in Envoy which
 * were converted to RELEASE_ASSERT(foo == bar, "");
 *
 * The new style of release assert is of the form
 * RELEASE_ASSERT(foo == bar, "reason foo should actually be bar");
 * new uses of RELEASE_ASSERT should supply a verbose explanation of what went wrong.
 */
#define RELEASE_ASSERT(X, DETAILS) _ASSERT_IMPL(X, #X, ::abort(), DETAILS)

 /**
  * Assert macro intended for Envoy Mobile. It creates enforcement for mobile
  * clients but has no effect for Envoy as a server.
  */
#if TARGET_OS_IOS || defined(__ANDROID_API__)
#define MOBILE_RELEASE_ASSERT(X, DETAILS) RELEASE_ASSERT(X, DETAILS)
#else
#define MOBILE_RELEASE_ASSERT(X, DETAILS)
#endif

  /**
   * Assert macro intended for security guarantees. It has the same functionality
   * as RELEASE_ASSERT, but is intended for memory bounds-checking.
   */
#define SECURITY_ASSERT(X, DETAILS) _ASSERT_IMPL(X, #X, ::abort(), DETAILS)

   // ENVOY_LOG_DEBUG_ASSERT_IN_RELEASE compiles all ASSERTs in release mode.
#ifdef ENVOY_LOG_DEBUG_ASSERT_IN_RELEASE
#define ENVOY_LOG_FAST_DEBUG_ASSERT_IN_RELEASE
#endif

#if !defined(NDEBUG) || defined(ENVOY_LOG_DEBUG_ASSERT_IN_RELEASE) ||                              \
    defined(ENVOY_LOG_FAST_DEBUG_ASSERT_IN_RELEASE)
// This if condition represents any case where ASSERT()s are compiled in.

#if !defined(NDEBUG) // If this is a debug build.
#define ASSERT_ACTION ::abort()
#else // If this is not a debug build, but ENVOY_LOG_(FAST)_DEBUG_ASSERT_IN_RELEASE is defined.
#define ASSERT_ACTION                                                                              \
  Envoy::Assert::invokeDebugAssertionFailureRecordActionForAssertMacroUseOnly(                     \
      __FILE__ ":" TOSTRING(__LINE__))
#endif // !defined(NDEBUG)

#define _ASSERT_ORIGINAL(X) _ASSERT_IMPL(X, #X, ASSERT_ACTION, "")
#define _ASSERT_VERBOSE(X, Y) _ASSERT_IMPL(X, #X, ASSERT_ACTION, Y)
#define _ASSERT_SELECTOR(_1, _2, ASSERT_MACRO, ...) ASSERT_MACRO

// This is a workaround for fact that MSVC expands __VA_ARGS__ after passing them into a macro,
// rather than before passing them into a macro. Without this, _ASSERT_SELECTOR does not work
// correctly when compiled with MSVC
#define EXPAND(X) X

#if !defined(ENVOY_DISABLE_KNOWN_ISSUE_ASSERTS)
/**
 * Assert wrapper for an as-yet unidentified issue. Even with ASSERTs compiled in, it may be
 * excluded, by defining ENVOY_DISABLE_KNOWN_ISSUE_ASSERTS. It represents a condition that
 * should always pass but that sometimes fails for an unknown reason. The macro allows it to
 * be temporarily compiled out while the failure is triaged and investigated.
 */
#define KNOWN_ISSUE_ASSERT(X, DETAILS) _ASSERT_IMPL(X, #X, ::abort(), DETAILS)
#else
// This non-implementation ensures that its argument is a valid expression that can be statically
// casted to a bool, but the expression is never evaluated and will be compiled away.
#define KNOWN_ISSUE_ASSERT _NULL_ASSERT_IMPL
#endif // defined(ENVOY_DISABLE_KNOWN_ISSUE_ASSERTS)

// If ASSERT is called with one argument, the ASSERT_SELECTOR will return
// _ASSERT_ORIGINAL and this will call _ASSERT_ORIGINAL(__VA_ARGS__).
// If ASSERT is called with two arguments, ASSERT_SELECTOR will return
// _ASSERT_VERBOSE, and this will call _ASSERT_VERBOSE,(__VA_ARGS__)
#define ASSERT(...)                                                                                \
  EXPAND(_ASSERT_SELECTOR(__VA_ARGS__, _ASSERT_VERBOSE, _ASSERT_ORIGINAL)(__VA_ARGS__))

#if !defined(NDEBUG) || defined(ENVOY_LOG_DEBUG_ASSERT_IN_RELEASE)
// debug build or all ASSERTs compiled in release.
#define SLOW_ASSERT(...) ASSERT(__VA_ARGS__)
#else
// Non-implementation of SLOW_ASSERTs when building only ENVOY_LOG_FAST_DEBUG_ASSERT_IN_RELEASE.
#define SLOW_ASSERT _NULL_ASSERT_IMPL
#endif // !defined(NDEBUG) || defined(ENVOY_LOG_DEBUG_ASSERT_IN_RELEASE)

#else
#define ASSERT _NULL_ASSERT_IMPL
#define KNOWN_ISSUE_ASSERT _NULL_ASSERT_IMPL
#define SLOW_ASSERT _NULL_ASSERT_IMPL
#endif // !defined(NDEBUG) || defined(ENVOY_LOG_DEBUG_ASSERT_IN_RELEASE) ||
	   // defined(ENVOY_LOG_FAST_DEBUG_ASSERT_IN_RELEASE)

/**
 * Indicate a panic situation and exit.
 */
#define PANIC(X)                                                                                   \
  do {                                                                                             \
    ENVOY_LOG_TO_LOGGER(Envoy::Logger::Registry::getLog(Envoy::Logger::Id::assert), critical,      \
                        "panic: {}", X);                                                           \
    ::abort();                                                                                     \
  } while (false)

 // We do not want to crash on failure in tests exercising ENVOY_BUGs while running coverage in debug
 // mode. Crashing causes flakes when forking to expect a debug death and reduces lines of coverage.
#if !defined(NDEBUG) && !defined(ENVOY_CONFIG_COVERAGE)
#define ENVOY_BUG_ACTION ::abort()
#else
#define ENVOY_BUG_ACTION                                                                           \
  Envoy::Assert::invokeEnvoyBugFailureRecordActionForEnvoyBugMacroUseOnly(__FILE__                 \
                                                                          ":" TOSTRING(__LINE__))
#endif

// These macros are needed to stringify __LINE__ correctly.
#define STRINGIFY(X) #X
#define TOSTRING(X) STRINGIFY(X)

// CONDITION_STR is needed to prevent macros in condition from being expected, which obfuscates
// the logged failure, e.g., "EAGAIN" vs "11".
// ENVOY_BUG logging and actions are invoked only on power-of-two instances per log line.
#define _ENVOY_BUG_IMPL(CONDITION, CONDITION_STR, ACTION, DETAILS)                                 \
  do {                                                                                             \
    if (!(CONDITION) && Envoy::Assert::shouldLogAndInvokeEnvoyBugForEnvoyBugMacroUseOnly(          \
                            __FILE__ ":" TOSTRING(__LINE__))) {                                    \
      const std::string& details = (DETAILS);                                                      \
      ENVOY_LOG_TO_LOGGER(Envoy::Logger::Registry::getLog(Envoy::Logger::Id::envoy_bug), error,    \
                          "envoy bug failure: {}.{}{}", CONDITION_STR,                             \
                          details.empty() ? "" : " Details: ", details);                           \
      Envoy::Assert::EnvoyBugStackTrace st;                                                        \
      st.capture();                                                                                \
      st.logStackTrace();                                                                          \
      ACTION;                                                                                      \
    }                                                                                              \
  } while (false)

#define _ENVOY_BUG_VERBOSE(X, Y) _ENVOY_BUG_IMPL(X, #X, ENVOY_BUG_ACTION, Y)

// This macro is needed to help to remove: "warning C4003: not enough arguments for function-like
// macro invocation '<identifier>'" when expanding __VA_ARGS__. In our setup, MSVC treats this
// warning as an error. A sample code to reproduce the case: https://godbolt.org/z/M4zZNG.
#define PASS_ON(...) __VA_ARGS__

/**
 * Indicate a failure condition that should never be met in normal circumstances. In contrast
 * with ASSERT, an ENVOY_BUG is compiled in release mode. If a failure condition is met in release
 * mode, it is logged and a stat is incremented with exponential back-off per ENVOY_BUG. In debug
 * mode, it will crash if the condition is not met. ENVOY_BUG must be called with two arguments for
 * verbose logging.
 * Note: ENVOY_BUGs in coverage mode will never crash. They will log and increment a stat like in
 * release mode. This prevents flakiness and increases code coverage.
 */
#define ENVOY_BUG(...) PASS_ON(PASS_ON(_ENVOY_BUG_VERBOSE)(__VA_ARGS__))

 // Always triggers ENVOY_BUG. This is intended for paths that are not expected to be reached.
#define IS_ENVOY_BUG(...) ENVOY_BUG(false, __VA_ARGS__);

// It is safer to avoid defaults in switch statements, so that as new enums are added, the compiler
// checks that new code is added as well. Google's proto library adds 2 sentinel values which should
// not be used, and this macro allows avoiding using "default:" to handle them.
#define PANIC_ON_PROTO_ENUM_SENTINEL_VALUES                                                        \
  case std::numeric_limits<int32_t>::max():                                                        \
    FALLTHRU;                                                                                      \
  case std::numeric_limits<int32_t>::min():                                                        \
    PANIC("unexpected sentinel value used")

#define PANIC_DUE_TO_PROTO_UNSET PANIC("unset oneof")

// Envoy has a number of switch statements which panic if there's no legal value set.
// This is not encouraged, as it's too easy to panic using break; instead of return;
// but this macro replaces a less clear crash using NOT_REACHED_GCOVR_EXCL_LINE.
#define PANIC_DUE_TO_CORRUPT_ENUM PANIC("corrupted enum");

	} // namespace Assert
} // namespace Envoy























#ifndef PSTATIC_ASSERT_H
#define PSTATIC_ASSERT_H

#include <assert.h>

/* Handle clang */
#ifndef __has_feature
#define __has_feature(x) 0
#endif

#if defined(static_assert)
#ifndef __static_assert_is_defined
#define __static_assert_is_defined 1
#endif
#endif

/* Handle static_assert as a keyword in C++ and compiler specifics. */
#if !defined(__static_assert_is_defined)

#if defined(__cplusplus)

#if __cplusplus >= 201103L
#define __static_assert_is_defined 1
#elif __has_feature(cxx_static_assert)
#define __static_assert_is_defined 1
#elif defined(_MSC_VER) && (_MSC_VER >= 1600)
#define __static_assert_is_defined 1
#endif

#else

#if defined(_MSC_VER) && (_MSC_VER >= 1600)
#define __static_assert_is_defined 1
#elif __has_feature(c_static_assert)
#define static_assert(pred, msg) _Static_assert(pred, msg)
#define __static_assert_is_defined 1
#elif defined (__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
/* In case the clib headers are not compliant. */
#define static_assert(pred, msg) _Static_assert(pred, msg)
#define __static_assert_is_defined 1
#endif

#endif /* __cplusplus */
#endif /* __static_assert_is_defined */


#if !defined(__static_assert_is_defined)

#define __PSTATIC_ASSERT_CONCAT_(a, b) static_assert_scope_##a##_line_##b
#define __PSTATIC_ASSERT_CONCAT(a, b) __PSTATIC_ASSERT_CONCAT_(a, b)
#ifdef __COUNTER__
#define static_assert(e, msg) enum { __PSTATIC_ASSERT_CONCAT(__COUNTER__, __LINE__) = 1/(!!(e)) }
#else
#include "pstatic_assert_scope.h"
#define static_assert(e, msg) enum { __PSTATIC_ASSERT_CONCAT(__PSTATIC_ASSERT_COUNTER, __LINE__) = 1/(int)(!!(e)) }
#endif

#define __static_assert_is_defined 1

#endif /* __static_assert_is_defined */

#endif /* PSTATIC_ASSERT_H */

/* Update scope counter outside of include guard. */
#ifdef __PSTATIC_ASSERT_COUNTER
#include "pstatic_assert_scope.h"
#endif


























#endif


