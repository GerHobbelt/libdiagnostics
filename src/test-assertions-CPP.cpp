
#pragma once

#include <diagnostics/assertions.h>

#include <iostream>
#include <fstream>

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
	//
	// --> error C2338: static_assert failed: 'A and B must be layout compatible'
#if defined(COMPILE_TIME_TEST)
	DIAG_ASSERT_EQ(fbar(), 123);
	DIAG_ASSERT_NE(fbar(), 5);
#endif

	std::string x{"foo"};
	std::string y{"foo"};

	libdiag::StringComparator comparator{};

	DIAG_ASSERT_STR_EQ(x, y);
	DIAG_ASSERT_STR_EQ(x, "foo");

	DIAG_ASSERT_COMPARATOR(comparator, "bar", x);

	libdiag::IEEE754Comparator flt_comparator{1e-5};
	std::cerr << "Comparing floats with epsilon: " << flt_comparator.nameof() << std::endl;
	DIAG_ASSERT_COMPARATOR(flt_comparator, fbar(), 123);

	{
		libdiag::IEEE754Comparator flt_comparator2({.relative_epsilon = 1e-4}); // https://pdimov.github.io/blog/2020/09/07/named-parameters-in-c20/
		libdiag::IEEE754Comparator flt_comparator3{0.1_pct};
		libdiag::IEEE754Comparator flt_comparator4{1e-6_perunage};
		libdiag::IEEE754Comparator flt_comparator5(5.0_promille, 0.0);
		libdiag::IEEE754Comparator flt_comparator6(0.1, 0.0);

		std::cerr << "Testing FP comparators:\n"
			<< ":: " << flt_comparator2.nameof() << "\n"
			<< ":: " << flt_comparator3.nameof() << "\n"
			<< ":: " << flt_comparator4.nameof() << "\n"
			<< ":: " << flt_comparator5.nameof() << "\n"
			<< ":: " << flt_comparator6.nameof() << "\n";

		DIAG_ASSERT_EQ(flt_comparator.parameters().absolute_epsilon, 1e-5);
		DIAG_ASSERT_EQ(flt_comparator.parameters().relative_epsilon, 1e-7);

		DIAG_ASSERT_EQ(flt_comparator2.parameters().absolute_epsilon, 1e-35);
		DIAG_ASSERT_EQ(flt_comparator2.parameters().relative_epsilon, 1e-4);

		DIAG_ASSERT_EQ(flt_comparator3.parameters().absolute_epsilon, 1e-35);
		DIAG_ASSERT_EQ(flt_comparator3.parameters().relative_epsilon, 0.1 / 100);

		DIAG_ASSERT_EQ(flt_comparator4.parameters().absolute_epsilon, 1e-35);
		DIAG_ASSERT_EQ(flt_comparator4.parameters().relative_epsilon, 1e-6);

		DIAG_ASSERT_EQ(flt_comparator5.parameters().absolute_epsilon, 0.0);
		DIAG_ASSERT_EQ(flt_comparator5.parameters().relative_epsilon, 5e-3);

		DIAG_ASSERT_EQ(flt_comparator6.parameters().absolute_epsilon, 0.1);
		DIAG_ASSERT_EQ(flt_comparator6.parameters().relative_epsilon, 0.0);

		DIAG_ASSERT_COMPARATOR(flt_comparator2, fbar(), 123.0 - 1e-3); // should pass, as the error introduced is *relative* and without bounds.
		DIAG_ASSERT_COMPARATOR(flt_comparator6, fbar(), 123 + 0.098);
		DIAG_ASSERT_COMPARATOR(flt_comparator6, fbar(), 123 - 0.11);  // should fail, as the error introduced is *absolute* and larger than the allowed epsilon.
	}

	DIAG_ASSERT_FLT_EQ_APPROX(fbar(), fbar() + 1e-3, 1e-2_perunage);

	DIAG_ASSERT_FLT_EQ_APPROX(fbar(), fbar() + 1e-3, libdiag::IEEE754Comparator::parameters_t({ .absolute_epsilon = 0.0, .relative_epsilon = 1e-4 })); // needs to `parameters` type to avoid ambiguity with the last comma between the two epsilon parameters

	libdiag::IEEE754Comparator flt_comparator_EX{{ .absolute_epsilon = 0.0, .relative_epsilon = 1e-4 }};

	// ... but, for readability and conciseness, you can also pass in a pre-initialized IEEE754Comparator object instead:

	DIAG_ASSERT_FLT_EQ_APPROX(fbar(), fbar() + 1e-3, flt_comparator_EX);


}


