// Basic algorithms.


#pragma once
#include "BF/Definitions.hpp"


namespace BF {


// === Contains() ======================================================================================================

template <class Range, class Type>
constexpr bool Contains(const Range& range, const Type& value)
{
	for (const auto& x : range) {
		if (x == value)
			return true;
	}

	return false;
}


}	// namespace BF
