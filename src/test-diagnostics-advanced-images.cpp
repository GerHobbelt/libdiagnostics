
#pragma once

#include <diagnostics/diagnostics.h>
#include <spdlog/spdlog.h>
#include <fmt/format.h>




// short-circuit evaluation:




// https://www.reddit.com/r/C_Programming/comments/aaa9kg/can_i_always_expect_the_and_operators_to_short/
//
// As an important side note: The same is not true in C++; whereas the original operators on built-in types do short-circuit, custom implementations do not have to, IIRC.

// In C++ it's actually not even possible for overloaded operator && or operator || to short-circuit — at least not without writing some complicated expression-template machinery for lazy evaluation.

// As others have mentioned, yes && and || are guaranteed by spec to short circuit, but as an interesting aside, the reason that there isn't a logical xor (^^) is that it can't short circuit.

// There is a logical xor. It's !=


// logical xor: one or the other but not both. should be able to? `A^^B == (A&&!B)||(B&&!A)` no?


// https://learn.microsoft.com/bg-bg/cpp/cpp/logical-and-operator-amp-amp?view=msvc-150

// https://en.cppreference.com/w/cpp/language/operators.html

// Restrictions
// - An operator function must have at least one function parameter or implicit object parameter whose type is a class, a reference to a class, an enumeration, or a reference to an enumeration.
// - The operators :: (scope resolution), . (member access), .* (member access through pointer to member), and ?: (ternary conditional) cannot be overloaded.
// - New operators such as **, <>, or &| cannot be created.
// - It is not possible to change the precedence, grouping, or number of operands of operators.
// - The overload of operator -> must either return a raw pointer, or return an object (by reference or by value) for which operator -> is in turn overloaded.
// - The overloads of operators && and || lose short-circuit evaluation.                 !! <============================== !!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
//  &&, ||, and , lose their special sequencing properties when overloaded and behave like regular function calls even when they are used without function-call notation.






/*

Function call operator
When a user-defined class overloads the function call operator operator(), it becomes a FunctionObject type.

An object of such a type can be used in a function call expression:

// An object of this type represents a linear function of one variable a * x + b.
struct Linear
{
	double a, b;

	double operator()(double x) const
	{
		return a * x + b;
	}
};

int main()
{
	Linear f{2, 1};  // Represents function 2x + 1.
	Linear g{-1, 0}; // Represents function -x.
	// f and g are objects that can be used like a function.

	double f_0 = f(0);
	double f_1 = f(1);

	double g_0 = g(0);
}
Many standard library algorithms accept FunctionObjects to customize behavior. There are no particularly notable canonical forms of operator(), but to illustrate the usage:

Run this code
#include <algorithm>
#include <iostream>
#include <vector>

struct Sum
{
	int sum = 0;
	void operator()(int n) { sum += n; }
};

int main()
{
	std::vector<int> v = {1, 2, 3, 4, 5};
	Sum s = std::for_each(v.begin(), v.end(), Sum());
	std::cout << "The sum is " << s.sum << '\n';
}


*/
