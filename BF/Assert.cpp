#include "BF/Assert.hpp"

#include <stdlib.h>


namespace BF {


void InitializeAssert()
{
	_set_error_mode(_OUT_TO_MSGBOX);
}


AssertInitializer::AssertInitializer()
{
	InitializeAssert();
}


}	// namespace BF
