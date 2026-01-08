#include "BF/Assert.hpp"

#include "gtest/gtest.h"
#include "BF/ClassUtils.hpp"
#include "BF/TestUtils.hpp"


// === Helpers =========================================================================================================

namespace {
	template <class...> struct True  { static constexpr bool Value = true;  };
	template <class...> struct False { static constexpr bool Value = false; };
}


// === Test cases ======================================================================================================

BF_COMPILE_TIME_TEST()
{
	{ bool                t = true, f = false; BF_ASSERT(t);           BF_VERIFY(t);           BF_ERROR(f);           }
	{ const bool          t = true, f = false; BF_ASSERT(t);           BF_VERIFY(t);           BF_ERROR(f);           }
	{ volatile bool       t = true, f = false; BF_ASSERT(t);           BF_VERIFY(t);           BF_ERROR(f);           }
	{ const volatile bool t = true, f = false; BF_ASSERT(t);           BF_VERIFY(t);           BF_ERROR(f);           }
	{ bool                t = true, f = false; BF_ASSERT(BF::Move(t)); BF_VERIFY(BF::Move(t)); BF_ERROR(BF::Move(f)); }
	{ const bool          t = true, f = false; BF_ASSERT(BF::Move(t)); BF_VERIFY(BF::Move(t)); BF_ERROR(BF::Move(f)); }
	{ volatile bool       t = true, f = false; BF_ASSERT(BF::Move(t)); BF_VERIFY(BF::Move(t)); BF_ERROR(BF::Move(f)); }
	{ const volatile bool t = true, f = false; BF_ASSERT(BF::Move(t)); BF_VERIFY(BF::Move(t)); BF_ERROR(BF::Move(f)); }

	static_assert(std::is_same_v<decltype(BF_VERIFY(true)), bool>);
	static_assert(std::is_same_v<decltype(BF_ERROR(true)),  bool>);

	BF_BREAK();
}


TEST(Assert, Good)
{
	BF_ASSERT(true);
	BF_ASSERT(1 + 1 == 2);
	BF_ASSERT(True<void, void>::Value);
	BF_ASSERT(std::initializer_list<int>{1, 2, 3}.size() == 3);

	if (false)
		BF_BREAK();

	BF_VERIFY(true);
	BF_VERIFY(1 + 1 == 2);
	BF_VERIFY(True<void, void>::Value);
	BF_VERIFY(std::initializer_list<int>{1, 2, 3}.size() == 3);

	BF_ERROR(false);
	BF_ERROR(1 + 1 != 2);
	BF_ERROR(False<void, void>::Value);
	BF_ERROR(std::initializer_list<int>{1, 2, 3}.size() != 3);

#if BF_DEBUG
	EXPECT_EQ(BF_VERIFY(true),  true);
	EXPECT_EQ(BF_ERROR(false),  false);
#else
	EXPECT_EQ(BF_VERIFY(true),  true);
	EXPECT_EQ(BF_VERIFY(false), false);
	EXPECT_EQ(BF_ERROR(true),   true);
	EXPECT_EQ(BF_ERROR(false),  false);
#endif
}


BF_COMPILE_TIME_TEST()
{
//	BF_ASSERT();					// [CompilationError-Debug]: ImpAssert::EnsureOneBoolParameter
//	BF_ASSERT(1);					// [CompilationError-Debug]: ImpAssert::EnsureOneBoolParameter
//	BF_ASSERT(BF::ImmobileClass());	// [CompilationError-Debug]: ImpAssert::EnsureOneBoolParameter
//	BF_ASSERT(true, true);			// [CompilationError-Debug]: ImpAssert::EnsureOneBoolParameter
//	BF_ASSERT(true, "message");		// [CompilationError-Debug]: ImpAssert::EnsureOneBoolParameter

//	BF_BREAK(0);					// [CompilationError]: too many arguments for function-like macro invocation

//	BF_VERIFY();					// [CompilationError]: ImpAssert::EnsureOneBoolParameter
//	BF_VERIFY(1);					// [CompilationError]: ImpAssert::EnsureOneBoolParameter
//	BF_VERIFY(BF::ImmobileClass());	// [CompilationError]: ImpAssert::EnsureOneBoolParameter
//	BF_VERIFY(true, true);			// [CompilationError]: ImpAssert::EnsureOneBoolParameter
//	BF_VERIFY(true, "message");		// [CompilationError]: ImpAssert::EnsureOneBoolParameter

//	BF_ERROR();						// [CompilationError]: ImpAssert::EnsureOneBoolParameter
//	BF_ERROR(1);					// [CompilationError]: ImpAssert::EnsureOneBoolParameter
//	BF_ERROR(BF::ImmobileClass());	// [CompilationError]: ImpAssert::EnsureOneBoolParameter
//	BF_ERROR(true, true);			// [CompilationError]: ImpAssert::EnsureOneBoolParameter
//	BF_ERROR(true, "message");		// [CompilationError]: ImpAssert::EnsureOneBoolParameter
}


BF_COMPILE_TIME_TEST()
{
//	BF_ASSERT;						// [CompilationError]: undeclared identifier
//	BF_BREAK;						// [CompilationError]: undeclared identifier
//	BF_VERIFY;						// [CompilationError]: undeclared identifier
//	BF_ERROR;						// [CompilationError]: undeclared identifier

//	BF_ASSERT(true)					// [CompilationError]: missing ';'
//	BF_BREAK()						// [CompilationError]: missing ';'
//	BF_VERIFY(true)					// [CompilationError]: missing ';'
//	BF_ERROR(false)					// [CompilationError]: missing ';'
}


#if BF_DEBUG
	constexpr int  BF_DUMMY = [] { BF_ASSERT(true);  return 0; }();
//	constexpr int  BF_DUMMY = [] { BF_ASSERT(false); return 0; }();		// [CompilationError-Debug]: expression did not evaluate to a constant
//	constexpr int  BF_DUMMY = [] { BF_BREAK();       return 0; }();		// [CompilationError-Debug]: expression did not evaluate to a constant
	constexpr bool BF_DUMMY = BF_VERIFY(true);
//	constexpr bool BF_DUMMY = BF_VERIFY(false);							// [CompilationError-Debug]: expression did not evaluate to a constant
//	constexpr bool BF_DUMMY = BF_ERROR(true);							// [CompilationError-Debug]: expression did not evaluate to a constant
	constexpr bool BF_DUMMY = BF_ERROR(false);
#else
	constexpr int  BF_DUMMY = [] { BF_ASSERT(true);  return 0; }();
	constexpr int  BF_DUMMY = [] { BF_ASSERT(false); return 0; }();
	constexpr int  BF_DUMMY = [] { BF_BREAK();       return 0; }();
	constexpr bool BF_DUMMY = BF_VERIFY(true);
	constexpr bool BF_DUMMY = BF_VERIFY(false);
	constexpr bool BF_DUMMY = BF_ERROR(true);
	constexpr bool BF_DUMMY = BF_ERROR(false);
#endif
