
#pragma once

#include <diagnostics/assertions.h>

#include <float.h>
#include <math.h>
#include <stdbool.h>


static int foo(void) {
	return 123;
}

static float fbar(void) {
	return 123.0f;
}

int main(void) {
	DIAG_ASSERT_FOR_EACH(123 == 123, 2 + 2 == 4, "foo"[1] == 'o');

	/*
	 * If we combine multiple assertions with the && operator, we will not be able to distinguish
	 * them if one of them fails apparently:
	 *
	 * main: Assertion `123 == 321 && 2 + 2 == 4 && "foo"[1] == 'o' failed.
	 * assert(123 == 321 && 2 + 2 == 4 && "foo"[1] == 'o');
	 */

	/*
	 * ... unlike `ASSERT_FOR_EACH` telling us which one has failed:
	 *
	 * main: Assertion `123 == 321' failed.
	 * ASSERT_FOR_EACH(123 == 321, 2 + 2 == 4, "foo"[1] == 'o');
	 */

	DIAG_ASSERT(foo() == 123);
	DIAG_ASSERT(fbar() == 123);
	DIAG_ASSERT(foo());
	DIAG_ASSERT(fbar());

	LIBDIAG_NULL_ASSERT_IMPL(123 == 123);
	LIBDIAG_NULL_ASSERT_IMPL(foo() == 123);

	DIAG_ASSERT_EQ(foo(), 123);
	DIAG_ASSERT_EQ(fbar(), 123.0f);

	DIAG_ASSERT_NE(fbar(), 5.0f);

	// comparing non-identical value types: double <-> integer
	DIAG_ASSERT_EQ(fbar(), 123);
	DIAG_ASSERT_NE(fbar(), 5);

	char x[] = "foo";
	char y[] = "foo";
	const char* z = "bar";

	struct StringComparatorSpec comparator = mk_StringComparator();

	DIAG_ASSERT_STR_EQ(x, y);
	DIAG_ASSERT_COMPARATOR(x, y, get_common_StringComparator(), "(superfluous extra message:) we wish you to inform we ran an assertion check with extra info string z: \"%s\" and fbar value: %lf", z, fbar());
	DIAG_ASSERT_STR_EQ(x, y, "(superfluous extra message:) we wish you to inform we ran an assertion check with extra info string z: \"%s\" and fbar value: %lf", z, fbar());
	DIAG_ASSERT_STR_EQ(x, "foo");

	DIAG_ASSERT_COMPARATOR("bar", x, &comparator);

	DIAG_ASSERT_FLT_EQ_APPROX_PERUNAGE(fbar(), 123.0 + 0.1, 1e-4);
	DIAG_ASSERT_FLT_EQ_APPROX_FIXED(fbar(), 123.0 + 0.1, 0.1);

	DIAG_ASSERT_FLT_EQ_APPROX_PERUNAGE(fbar(), 123.0 + 0.1, 1e-5);
	DIAG_ASSERT_FLT_EQ_APPROX_FIXED(fbar(), 123.0 + 0.1, 1e-5);

	//libdiag::IEEE754Comparator flt_comparator{1e-5};
	struct IEEE754ComparatorSpec flt_comparator = mk_IEEE754Comparator(1e-5, -1.0);

	//std::cerr << "Comparing floats with epsilon: " << flt_comparator.nameof() << std::endl;
	DIAG_ASSERT_COMPARATOR(fbar(), 123, &flt_comparator);

	DIAG_ASSERT_FLT_EQ_APPROX(fbar(), fbar() + 1e-3);
	DIAG_ASSERT_FLT_EQ_APPROX(fbar(), fbar() + 1e-3, "(superfluous extra message:) we wish you to inform we ran an assertion check with extra info string z: \"%s\" and fbar value: %lf", z, fbar());

	const struct IEEE754ComparatorParameters cmp_settings = { .absolute_epsilon = 0.0, .relative_epsilon = 1e-4 };
	struct IEEE754ComparatorSpec flt_comparator_EX = mk_IEEE754Comparator_alt(&cmp_settings);

	DIAG_ASSERT_FLT_EQ_APPROX_EX(fbar(), fbar() + 1e-3, &flt_comparator_EX);
	DIAG_ASSERT_FLT_EQ_APPROX_EX(fbar(), fbar() + 1e-3, &flt_comparator_EX, "(superfluous extra message:) we wish you to inform we ran an assertion check with extra info string z: \"%s\" and fbar value: %lf", z, fbar());

	return 0;
}


