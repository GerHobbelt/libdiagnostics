
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



