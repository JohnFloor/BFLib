#include "BF/Definitions.hpp"

#include <limits>
#include <type_traits>
#include "gtest/gtest.h"
#include "BF/TestUtils.hpp"


// === Fixed width integer types =======================================================================================

template <class Alias, auto MaxAlias, class AliasedType>
static void TestIntegerAlias()
{
	static_assert(std::is_same_v<Alias, AliasedType>);

	static_assert(std::is_same_v<decltype(MaxAlias), Alias>);
	static_assert(MaxAlias == std::numeric_limits<Alias>::max());
}


BF_COMPILE_TIME_TEST()
{
	TestIntegerAlias<Int8,    MaxInt8,    std::int8_t   >();
	TestIntegerAlias<Int16,   MaxInt16,   std::int16_t  >();
	TestIntegerAlias<Int32,   MaxInt32,   std::int32_t  >();
	TestIntegerAlias<Int64,   MaxInt64,   std::int64_t  >();
	TestIntegerAlias<IntPtr,  MaxIntPtr,  std::intptr_t >();

	TestIntegerAlias<UInt8,   MaxUInt8,   std::uint8_t  >();
	TestIntegerAlias<UInt16,  MaxUInt16,  std::uint16_t >();
	TestIntegerAlias<UInt32,  MaxUInt32,  std::uint32_t >();
	TestIntegerAlias<UInt64,  MaxUInt64,  std::uint64_t >();
	TestIntegerAlias<UIntPtr, MaxUIntPtr, std::uintptr_t>();
}


// === Other type aliases ==============================================================================================

BF_COMPILE_TIME_TEST()
{
	TestIntegerAlias<UChar, MaxUChar, unsigned char>();
}


// === BF_DEBUG, BF_RELEASE ============================================================================================

static_assert(std::is_same_v<decltype(BF_DEBUG),   bool>);
static_assert(std::is_same_v<decltype(BF_RELEASE), bool>);

#ifdef NDEBUG
constexpr bool NDebugDefined = true;
#else
constexpr bool NDebugDefined = false;
#endif

static_assert(BF_DEBUG   != NDebugDefined);
static_assert(BF_RELEASE == NDebugDefined);

#if BF_DEBUG
// Debug-only code...
#endif

#if BF_RELEASE
// Release-only code...
#endif


// === BF_UNUSED_VAR ===================================================================================================

BF_COMPILE_TIME_TEST(int par)
{
	int local;
	BF_UNUSED_VAR(local, par);
}


// === BF_NOINLINE =====================================================================================================

BF_NOINLINE static void NotInlinedFunction()
{
}


BF_COMPILE_TIME_TEST()
{
	NotInlinedFunction();
}


// === BF_IMPLIES ======================================================================================================

static constexpr bool TestImplies(bool a, bool b)
{
	const bool test      = a BF_IMPLIES b;
	const bool reference = !a || b;

	return test == reference;
}


static_assert(TestImplies(false, false));
static_assert(TestImplies(false, true));
static_assert(TestImplies(true,  false));
static_assert(TestImplies(true,  true));


// === BF_FWD ==========================================================================================================

BF_COMPILE_TIME_TEST()
{
	int i0 = 0;

	int         i    = 0;
	int&        ir   = i0;
	int&&       irr  = 0;
	const int   ci   = 0;
	const int&  cir  = 0;
	const int&& cirr = 0;

	BF::AssertIsRValue(BF_FWD(i));
	BF::AssertIsLValue(BF_FWD(ir));
	BF::AssertIsRValue(BF_FWD(irr));
	BF::AssertIsRValue(BF_FWD(ci));
	BF::AssertIsLValue(BF_FWD(cir));
	BF::AssertIsRValue(BF_FWD(cirr));

	int& ii = i;
	// If BF_FWD() is implemented with std::forward (and not static_cast), IntelliSense correctly marks the following
	// line as "C26800 Use of a moved from object: 'i' (lifetime.1).". Therefore it is implemented with std::forward.
//	int ignore = BF_FWD(ii);
}


// === BF_STRINGIFY ====================================================================================================

static_assert(BF_STRINGIFY(abcd)[0] == 'a');
static_assert(BF_STRINGIFY(abcd)[1] == 'b');
static_assert(BF_STRINGIFY(abcd)[2] == 'c');
static_assert(BF_STRINGIFY(abcd)[3] == 'd');

#define ABCD	abcd
static_assert(BF_STRINGIFY(ABCD)[0] == 'a');
static_assert(BF_STRINGIFY(ABCD)[1] == 'b');
static_assert(BF_STRINGIFY(ABCD)[2] == 'c');
static_assert(BF_STRINGIFY(ABCD)[3] == 'd');


// === BF_PASTE ========================================================================================================

static_assert(BF_PASTE(1111111, 2222222) == 11111112222222);

#define VALUE_1 1111111
#define VALUE_2 2222222
static_assert(BF_PASTE(VALUE_1, VALUE_2) == 11111112222222);


// === BF_COUNTED_NAME =================================================================================================

namespace {

class BF_COUNTED_NAME(MyPrefix) {};		class BF_COUNTED_NAME(MyPrefix) {};

}	// namespace


// === BF_DUMMY_NAME, BF_DUMMY =========================================================================================

namespace {


namespace BF_DUMMY_NAME {
	class BF_DUMMY_NAME {};			class BF_DUMMY_NAME {};
	enum  BF_DUMMY_NAME {};			enum  BF_DUMMY_NAME {};
	using BF_DUMMY_NAME = void;		using BF_DUMMY_NAME = void;
}


int BF_DUMMY;	int BF_DUMMY;
int BF_DUMMY{1};
int BF_DUMMY(2);
int BF_DUMMY = 3;


enum BF_DUMMY_NAME { BF_DUMMY, BF_DUMMY };


class BF_DUMMY_NAME {
	int BF_DUMMY;
	int BF_DUMMY{1};
	int BF_DUMMY = 3;

	static const int BF_DUMMY;
	static const int BF_DUMMY{1};
	static const int BF_DUMMY = 3;
};


static void BF_DUMMY()
{
	int BF_DUMMY;
	int BF_DUMMY{1};
	int BF_DUMMY(2);
	int BF_DUMMY = 3;
}


}	// namespace


// === BF_SCOPE, BF_SCOPE_DECL =========================================================================================

namespace {


enum State {
	Initial,
	Constructed,
	Destroyed
};


class TestScope {
public:
	explicit TestScope(State& state) : mState(state) { mState = Constructed; }
	~TestScope()									 { mState = Destroyed;   }

private:
	State& mState;
};


}	// namespace


TEST(Definitions, BF_SCOPE)
{
	State s;

	s = Initial;
	BF_SCOPE(TestScope(s))								// if BF_SCOPE() has most vexing parse, 's' remains 'Initial'
		EXPECT_EQ(s, Constructed);
	EXPECT_EQ(s, Destroyed);

	s = Initial;
	BF_SCOPE(TestScope(s)) {							// if BF_SCOPE() has most vexing parse, 's' remains 'Initial'
		EXPECT_EQ(s, Constructed);
	}
	EXPECT_EQ(s, Destroyed);
}


TEST(Definitions, BF_SCOPE_DECL)
{
	State s;

	s = Initial;
	BF_SCOPE_DECL(TestScope scope(s))
		EXPECT_EQ(s, Constructed);
	EXPECT_EQ(s, Destroyed);

	s = Initial;
	BF_SCOPE_DECL(TestScope scope(s)) {
		EXPECT_EQ(s, Constructed);
	}
	EXPECT_EQ(s, Destroyed);

	int i = 0;
	BF_SCOPE_DECL(int i = 1) { EXPECT_EQ(i, 1); }		// hiding demo; no name collision between consecutive scopes
	BF_SCOPE_DECL(int i = 2) { EXPECT_EQ(i, 2); }		// hiding demo; no name collision between consecutive scopes
}


BF_COMPILE_TIME_TEST()
{
	BF_SCOPE(0) {
//	} else {											// [CompilationError]: illegal else without matching if
	}

	BF_SCOPE_DECL(int i = 0) {
//	} else {											// [CompilationError]: illegal else without matching if
	}
}


static int ControlPathsTest1()
{
	BF_SCOPE(0)
		return 0;										// not all control paths return a value, but should not warn
}


static int ControlPathsTest2()
{
	BF_SCOPE_DECL(int i = 0)
		return 0;										// not all control paths return a value, but should not warn
}


// === Bad values ======================================================================================================

static_assert(std::is_same_v<decltype(BF::BadValue16),  const UInt16>);
static_assert(std::is_same_v<decltype(BF::BadValue32),  const UInt32>);
static_assert(std::is_same_v<decltype(BF::BadValue64),  const UInt64>);
static_assert(std::is_same_v<decltype(BF::BadValuePtr), const UIntPtr>);

static_assert((BF::BadValue16 & 0xFF00) != 0x00 && (BF::BadValue16 & 0x00FF) != 0x00);
static_assert(BF::BadValue32  == (UInt32(BF::BadValue16) << 16) + BF::BadValue16);
static_assert(BF::BadValue64  == (UInt64(BF::BadValue32) << 32) + BF::BadValue32);
static_assert(BF::BadValuePtr == static_cast<UIntPtr>(BF::BadValue64));


// === Bad =============================================================================================================

static constexpr bool BadTest(BF::BadSelector) { return true; }
static_assert(BadTest(BF::Bad));
// static_assert(BadTest({}));							// [CompilationError]: cannot convert argument 1 from 'initializer list' to 'BF::BadSelector'


// === Uninitialized ===================================================================================================

static constexpr bool UninitializedTest(BF::UninitializedSelector) { return true; }
static_assert(UninitializedTest(BF::Uninitialized));
// static_assert(UninitializedTest({}));				// [CompilationError]: cannot convert argument 1 from 'initializer list' to 'BF::UninitializedSelector'
