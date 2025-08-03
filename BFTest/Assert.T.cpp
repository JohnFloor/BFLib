#include "BF/Assert.hpp"

#include "gtest/gtest.h"
#include "BF/ClassUtils.hpp"
#include "BF/TestUtils.hpp"


// === Helpers =========================================================================================================

namespace {
	template <class...>
	struct True {
		static constexpr bool Value = true;
	};
}


// === Test cases ======================================================================================================

BF_COMPILE_TIME_TEST()
{
	{ bool                b = true; BF_ASSERT(b); }
	{ const bool          b = true; BF_ASSERT(b); }
	{ volatile bool       b = true; BF_ASSERT(b); }
	{ const volatile bool b = true; BF_ASSERT(b); }
	{ bool                b = true; BF_ASSERT(BF::Move(b)); }
	{ const bool          b = true; BF_ASSERT(BF::Move(b)); }
	{ volatile bool       b = true; BF_ASSERT(BF::Move(b)); }
	{ const volatile bool b = true; BF_ASSERT(BF::Move(b)); }
}


TEST(Assert, Good)
{
	BF_ASSERT(true);
	BF_ASSERT(1 + 1 == 2);
	BF_ASSERT(True<void, void>::Value);
	BF_ASSERT(std::initializer_list<int>{1, 2, 3}.size() == 3);

	if (false)
		BF_BREAK();
}


BF_COMPILE_TIME_TEST()
{
	// These won't give the expected error message in Release build. In Release they will compile to ((void)0).
//	BF_ASSERT();					// [CompilationError-Debug]: ImpAssert::EnsureOneBoolParameter
//	BF_ASSERT(1);					// [CompilationError-Debug]: ImpAssert::EnsureOneBoolParameter
//	BF_ASSERT(BF::ImmobileClass());	// [CompilationError-Debug]: ImpAssert::EnsureOneBoolParameter
//	BF_ASSERT(true, true);			// [CompilationError-Debug]: ImpAssert::EnsureOneBoolParameter
//	BF_ASSERT(true, "message");		// [CompilationError-Debug]: ImpAssert::EnsureOneBoolParameter
}


BF_COMPILE_TIME_TEST()
{
//	BF_ASSERT;						// [CompilationError]: undeclared identifier
//	BF_BREAK;						// [CompilationError]: undeclared identifier
//	BF_ASSERT(true)					// [CompilationError]: missing ';' before '}'
//	BF_BREAK()						// [CompilationError]: missing ';' before '}'
}


static constexpr void ConstexprTest()
{
	BF_ASSERT(true);
}
