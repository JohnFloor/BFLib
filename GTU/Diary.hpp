// A class that keeps track of what happens to it. This can be later queried in tests.
// There is a separate record of events for every thread.


#pragma once
#include <string>
#include <string_view>
#include "BF/Definitions.hpp"


namespace GTU {		// Google Test Utilities


namespace ImpDiary {
	extern thread_local std::string gEvents;
}


class Diary {
public:
	Diary()								{ ImpDiary::gEvents += '+'; }
	Diary(const Diary&)					{ ImpDiary::gEvents += 'C'; }
	Diary(Diary&&) noexcept				{ ImpDiary::gEvents += 'M'; }
	Diary& operator=(const Diary&)		{ ImpDiary::gEvents += 'C'; return *this; }		// records self-copy too
	Diary& operator=(Diary&&) noexcept	{ ImpDiary::gEvents += 'M'; return *this; }		// records self-move too
	~Diary()							{ ImpDiary::gEvents += '-'; }

	class Guard {
	public:
		explicit Guard(std::string_view expectedEvents);
		~Guard();
	private:
		std::string mExpectedEvents;
	};
};


#define GTU_XD(expectedEvents)		BF_SCOPE(GTU::Diary::Guard(expectedEvents))


}	// namespace GTU
