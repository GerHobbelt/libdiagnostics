
#pragma once

#include <diagnostics/assertions.h>

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
	DIAG_ASSERT_EQ(fbar(), 123);

	DIAG_ASSERT_NE(fbar(), 5);
}


