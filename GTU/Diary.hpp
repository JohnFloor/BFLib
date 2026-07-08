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


inline void Push(char event)				{ ImpDiary::gEvents += event; }
inline void Push(std::string_view event)	{ ImpDiary::gEvents += event; }


class Diary {
public:
	Diary()									{ Push('+'); }
	Diary(const Diary&)						{ Push('C'); }
	Diary(Diary&&) noexcept					{ Push('M'); }
	Diary& operator=(const Diary&)			{ Push('C'); return *this; }		// records self-copy too
	Diary& operator=(Diary&&) noexcept		{ Push('M'); return *this; }		// records self-move too
	~Diary()								{ Push('-'); }

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
