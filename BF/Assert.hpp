// Assert macros.


#pragma once
#include <cassert>
#include "BF/Definitions.hpp"


namespace BF {


namespace ImpAssert {
	constexpr bool EnsureOneBoolParameter(bool par) { return par; }
	constexpr bool EnsureOneBoolParameter(auto&&...) = delete;
}


void InitializeAsserts();


class AssertInitializer {
public:
	AssertInitializer();
};


}	// namespace BF


#define BF_ASSERT(...)	assert(BF::ImpAssert::EnsureOneBoolParameter(__VA_ARGS__))
#define BF_BREAK()		assert(false)


#if BF_DEBUG
	#define BF_VERIFY(...)	(BF::ImpAssert::EnsureOneBoolParameter(__VA_ARGS__) ? (true)             : (BF_BREAK(), false))
	#define BF_ERROR(...)	(BF::ImpAssert::EnsureOneBoolParameter(__VA_ARGS__) ? (BF_BREAK(), true) : (false)            )
#else
	#define BF_VERIFY(...)	(BF::ImpAssert::EnsureOneBoolParameter(__VA_ARGS__))
	#define BF_ERROR(...)	(BF::ImpAssert::EnsureOneBoolParameter(__VA_ARGS__))
#endif
