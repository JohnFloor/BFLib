// Assert macros.


#pragma once
#include <cassert>
#include "BF/Definitions.hpp"


namespace ImpAssert {
	constexpr bool EnsureOneBoolParameter(bool par) { return par; }
	constexpr bool EnsureOneBoolParameter(auto&&...) = delete;
}


#define BF_ASSERT(...)	assert(ImpAssert::EnsureOneBoolParameter(__VA_ARGS__))
#define BF_BREAK()		assert(false)


#if BF_DEBUG
	#define BF_VERIFY(...)	(ImpAssert::EnsureOneBoolParameter(__VA_ARGS__) ? (true)             : (BF_BREAK(), false))
	#define BF_ERROR(...)	(ImpAssert::EnsureOneBoolParameter(__VA_ARGS__) ? (BF_BREAK(), true) : (false)            )
#else
	#define BF_VERIFY(...)	(ImpAssert::EnsureOneBoolParameter(__VA_ARGS__))
	#define BF_ERROR(...)	(ImpAssert::EnsureOneBoolParameter(__VA_ARGS__))
#endif
