#include "GTU/Diary.hpp"

#include "gtest/gtest.h"


namespace GTU {


thread_local std::string ImpDiary::gEvents;


Diary::Guard::Guard(std::string_view expectedEvents) :
	mExpectedEvents(expectedEvents)
{
	ImpDiary::gEvents.clear();
}


Diary::Guard::~Guard()
{
	EXPECT_EQ(ImpDiary::gEvents, mExpectedEvents);
}


}	// namespace GTU
