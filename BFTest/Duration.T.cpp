#include "BF/Duration.hpp"

#include <cstdio>
#include "BF/TestUtils.hpp"


// Not tested. Only usage examples are shown.


// === class HiResClock ================================================================================================

BF_COMPILE_TIME_TEST()
{
	BF::HiResClock::time_point t = BF::HiResClock::now();
}


// === MeasureDuration() ===============================================================================================

BF_COMPILE_TIME_TEST()
{
	double printfDuration = BF::MeasureDuration([] {			// seconds
		std::printf("Example line.\n");
	});
}


// === GetNothingDuration() ============================================================================================

BF_COMPILE_TIME_TEST()
{
	double nothingDuration = BF::GetNothingDuration();			// seconds
}


// === class Timer =====================================================================================================

BF_COMPILE_TIME_TEST()
{
	for (BF::Timer t(10.0); t.HasTime(); )
		std::printf("%f %%\n", t.GetProgress() * 100);
}
