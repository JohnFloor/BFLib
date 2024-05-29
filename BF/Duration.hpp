// Measuring duration.


#pragma once
#include <chrono>
#include "BF/Definitions.hpp"


namespace BF {


double MeasureDuration(auto&& callable)
{
	const auto t1 = std::chrono::high_resolution_clock::now();
	BF_FWD(callable)();
	const auto t2 = std::chrono::high_resolution_clock::now();

	return std::chrono::duration<double>(t2 - t1).count();		// seconds
}


double GetNothingDuration();


}	// namespace BF
