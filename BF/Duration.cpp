#include "BF/Duration.hpp"


namespace BF {


double GetNothingDuration()
{
	static double duration = 0.0;

	while (duration == 0.0)
		duration = MeasureDuration([] {});		// it can be exactly 0.0

	return duration;
}


}	// namespace BF
