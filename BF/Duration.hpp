// Measuring duration.


#pragma once
#include <chrono>
#include "BF/Assert.hpp"
#include "BF/ClassUtils.hpp"


namespace BF {


// === class HiResClock ================================================================================================

class HiResClock : public std::chrono::high_resolution_clock {
};


// === MeasureDuration() ===============================================================================================

double MeasureDuration(auto&& callable)
{
	const auto t1 = HiResClock::now();
	BF_FWD(callable)();
	const auto t2 = HiResClock::now();

	return std::chrono::duration<double>(t2 - t1).count();		// seconds
}


// === GetNothingDuration() ============================================================================================

double GetNothingDuration();


// === class Timer =====================================================================================================

class Timer : ImmobileClass {
public:
	explicit Timer(double seconds) :
		mSeconds(seconds),
		mEndTime(HiResClock::now() + std::chrono::nanoseconds(Int64(seconds * 1e9)))
	{
		BF_ASSERT(seconds >= 0.0);
	}

	bool HasTime() const {
		return HiResClock::now() < mEndTime;
	}

	double GetProgress() const {
		const double remainingDuration = std::chrono::duration<double>(mEndTime - HiResClock::now()).count();		// seconds
		const double progress          = 1.0 - remainingDuration / mSeconds;

		return progress <= 1.0 ? progress : 1.0;
	}

private:
	const double				 mSeconds;
	const HiResClock::time_point mEndTime;
};


}	// namespace BF
