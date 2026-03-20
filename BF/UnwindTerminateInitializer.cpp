#include "BF/UnwindTerminateInitializer.hpp"

#include <exception>


namespace BF {


void InitializeUnwindTerminate()
{
	std::set_terminate([] { std::exit(1010101); });
}


UnwindTerminateInitializer::UnwindTerminateInitializer()
{
	InitializeUnwindTerminate();
}


}	// namespace BF
