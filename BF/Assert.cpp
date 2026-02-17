#include "BF/Assert.hpp"

#include <stdlib.h>


namespace BF {


void InitializeAsserts()
{
	_set_error_mode(_OUT_TO_MSGBOX);
}


AssertInitializer::AssertInitializer()
{
	InitializeAsserts();
}


}	// namespace BF
