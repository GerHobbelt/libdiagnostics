
#pragma once

#include <diagnostics/logging.h>

#include <spdlog/spdlog.h>
#include <fmt/format.h>

#include <format>
#include <ostream>
#include <chrono>


// ---------------------------------------------------------------
// 

// you should define summartReport taking ostream& as parameter, as shown here:

std::ostream&  storageRentals::summaryReport(std::ostream & out) const
{
	//use out instead of cout
	for (int count = 0; count < 8; count++)
		out << "Unit: " << count + 1 << "    " << stoUnits[count] << endl;

	return out; //return so that op<< can be just one line!
}
then call it as:

ostream& operator <<(ostream& osObject, const storageRentals& rentals)
{
	return rentals.summaryReport(osObject); //just one line!
}
By the way, it is not called "overloading cout". You should say, "overloading operator<< for std::ostream.





m just telling you about one other possibility: I like using friend definitions for that:

	namespace Math
{
	class Matrix
	{
	public:

		[...]

			friend std::ostream& operator<< (std::ostream& stream, const Matrix& matrix) {
			[...]
		}
	};
}

The function will be automatically targeted into the surrounding namespace Math (even though its definition appears within
the scope of that class) but will not be visible unless you call operator<< with a Matrix object which will make argument
dependent lookup find that operator definition. That can sometimes help with ambiguous calls, since it's invisible for
argument types other than Matrix. When writing its definition, you can also refer directly to names defined in Matrix and
to Matrix itself, without qualifying the name with some possibly long prefix and providing template parameters like Math::Matrix<TypeA, N>.


