// Basic math utilities.


#pragma once
#include <type_traits>
#include "BF/Definitions.hpp"


namespace BF {


// === IPow() ==========================================================================================================

template <class Type>
constexpr Type IPow(Type base, UInt64 exponent)
{
	static_assert(std::is_integral_v<Type> && !std::is_same_v<Type, bool>, "'Type' should be an integer.");

	Type result = 1;
	Type x      = base;

	while (exponent > 0) {			// invariant: result * x^exponent
		if (exponent % 2 == 1)
			result *= x;

		x *= x;
		exponent /= 2;
	}

	return result;
}


// === IsPowerOf2() ====================================================================================================

constexpr bool IsPowerOf2(UInt64 n)
{
	return n != 0 && (n & (n - 1)) == 0;
}


}	// namespace BF
