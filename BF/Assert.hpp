// Assert macros.


#pragma once
#include <cassert>


namespace ImpAssert {
	constexpr bool EnsureOneBoolParameter(bool par) { return par; }
	constexpr bool EnsureOneBoolParameter(auto&&...) = delete;
}


#define BF_ASSERT(...)	assert(ImpAssert::EnsureOneBoolParameter(__VA_ARGS__))
#define BF_BREAK()		assert(false)
