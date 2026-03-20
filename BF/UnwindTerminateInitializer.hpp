// Makes std::terminate() call std::exit(), which unwinds the stack on exception.


#pragma once
#include "BF/Definitions.hpp"


namespace BF {


void InitializeUnwindTerminate();


class UnwindTerminateInitializer {
public:
	UnwindTerminateInitializer();
};


}	// namespace BF
