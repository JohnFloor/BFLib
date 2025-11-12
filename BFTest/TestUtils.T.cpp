#include "BF/TestUtils.hpp"

#include "gtest/gtest.h"


// === BF_DUMMY_NAME, BF_DUMMY =========================================================================================

namespace {


namespace BF_DUMMY_NAME {}
class BF_DUMMY_NAME {};
enum  BF_DUMMY_NAME {};
using BF_DUMMY_NAME = void;


int BF_DUMMY;
int BF_DUMMY{1};
int BF_DUMMY(2);
int BF_DUMMY = 3;


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


// === BF_COMPILE_TIME_TEST ============================================================================================

BF_COMPILE_TIME_TEST()
{
}

BF_COMPILE_TIME_TEST()
{
}

BF_COMPILE_TIME_TEST(int par1, int& par2, int&& par3)
{
}


// === AssertIsLValue(), AssertIsRValue() ==============================================================================

BF_COMPILE_TIME_TEST()
{
	int                i   = 0;
	const int          ci  = 0;
	volatile int       vi  = 0;
	const volatile int cvi = 0;

	BF::AssertIsLValue("");
	BF::AssertIsLValue(i);
	BF::AssertIsLValue(ci);
	BF::AssertIsLValue(vi);
	BF::AssertIsLValue(cvi);

	BF::AssertIsRValue(0);
	BF::AssertIsRValue(nullptr);
	BF::AssertIsRValue(BF::Move(i));
	BF::AssertIsRValue(BF::Move(ci));
	BF::AssertIsRValue(BF::Move(vi));
	BF::AssertIsRValue(BF::Move(cvi));

	BF::AssertIsRValue(static_cast<int>(i));
	BF::AssertIsRValue(static_cast<const int>(ci));
}


// === Move() ==========================================================================================================

constexpr int BF_DUMMY = BF::Move(0);
static_assert(noexcept(BF::Move(0)));


BF_COMPILE_TIME_TEST()
{
	int                i   = 0;
	const int          ci  = 0;
	volatile int       vi  = 0;
	const volatile int cvi = 0;

	static_assert(std::is_same_v<decltype(BF::Move(i)),              decltype(std::move(i))>);
	static_assert(std::is_same_v<decltype(BF::Move(ci)),             decltype(std::move(ci))>);
	static_assert(std::is_same_v<decltype(BF::Move(vi)),             decltype(std::move(vi))>);
	static_assert(std::is_same_v<decltype(BF::Move(cvi)),            decltype(std::move(cvi))>);

	static_assert(std::is_same_v<decltype(BF::Move(std::move(i))),   decltype(std::move(std::move(i)))>);
	static_assert(std::is_same_v<decltype(BF::Move(std::move(ci))),  decltype(std::move(std::move(ci)))>);
	static_assert(std::is_same_v<decltype(BF::Move(std::move(vi))),  decltype(std::move(std::move(vi)))>);
	static_assert(std::is_same_v<decltype(BF::Move(std::move(cvi))), decltype(std::move(std::move(cvi)))>);
}


// === LVal() ==========================================================================================================

template <class Type>
static void TestLVal()
{
	Type x{};

	static_assert(std::is_same_v<decltype(BF::LVal(x)),            Type&>);
	static_assert(std::is_same_v<decltype(BF::LVal(std::move(x))), Type&>);

	static_assert(&BF::LVal(x)            == &x);
	static_assert(&BF::LVal(std::move(x)) == &x);

	static_assert(noexcept(BF::LVal(x)));
	static_assert(noexcept(BF::LVal(std::move(x))));
}


BF_COMPILE_TIME_TEST()
{
	TestLVal<int>();
	TestLVal<const int>();
	TestLVal<volatile int>();
	TestLVal<const volatile int>();
}


// === PoisonedAddrOpClass =============================================================================================

template <class Type>
concept CanUseAddrOp = requires (Type value) { &BF_FWD(value); };

static_assert(!CanUseAddrOp<BF::PoisonedAddrOpClass&>);
static_assert(!CanUseAddrOp<BF::PoisonedAddrOpClass&&>);
static_assert(!CanUseAddrOp<const BF::PoisonedAddrOpClass&>);
static_assert(!CanUseAddrOp<const BF::PoisonedAddrOpClass&&>);
static_assert(!CanUseAddrOp<volatile BF::PoisonedAddrOpClass&>);
static_assert(!CanUseAddrOp<volatile BF::PoisonedAddrOpClass&&>);
static_assert(!CanUseAddrOp<const volatile BF::PoisonedAddrOpClass&>);
static_assert(!CanUseAddrOp<const volatile BF::PoisonedAddrOpClass&&>);


// === MemsetValue =====================================================================================================

TEST(TestUtils, MemsetValue)
{
	using TestType = BF::MemsetValue<int>;

	alignas(TestType) char buffer[sizeof(TestType)];
	TestType& value = reinterpret_cast<TestType&>(buffer);

	::new (buffer) TestType(123);
	EXPECT_EQ(value.GetL(),   123);
	EXPECT_EQ(value.GetCL(),  123);
	EXPECT_EQ(value.GetVL(),  123);
	EXPECT_EQ(value.GetCVL(), 123);
	EXPECT_EQ(value.GetR(),   123);
	EXPECT_EQ(value.GetCR(),  123);
	EXPECT_EQ(value.GetVR(),  123);
	EXPECT_EQ(value.GetCVR(), 123);
	value.~TestType();

	for (const UChar c : buffer)
		EXPECT_EQ(c, 0xBB);

	static_assert(std::is_base_of_v<BF::ImmobileClass, TestType>);

	static_assert(std::is_same_v<decltype(TestType{}.GetL()),   int&>);
	static_assert(std::is_same_v<decltype(TestType{}.GetCL()),  const int&>);
	static_assert(std::is_same_v<decltype(TestType{}.GetVL()),  volatile int&>);
	static_assert(std::is_same_v<decltype(TestType{}.GetCVL()), const volatile int&>);

	static_assert(std::is_same_v<decltype(TestType{}.GetR()),   int&&>);
	static_assert(std::is_same_v<decltype(TestType{}.GetCR()),  const int&&>);
	static_assert(std::is_same_v<decltype(TestType{}.GetVR()),  volatile int&&>);
	static_assert(std::is_same_v<decltype(TestType{}.GetCVR()), const volatile int&&>);
}
