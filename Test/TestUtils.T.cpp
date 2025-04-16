#include "BF/TestUtils.hpp"

#include "gtest/gtest.h"


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
[[gsl::suppress(es.56)]]		// allow std::move on constant variable
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
	BF::AssertIsRValue(std::move(i));
	BF::AssertIsRValue(std::move(ci));
	BF::AssertIsRValue(std::move(vi));
	BF::AssertIsRValue(std::move(cvi));

	BF::AssertIsRValue(static_cast<int>(i));
	BF::AssertIsRValue(static_cast<const int>(ci));
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
	EXPECT_EQ(123, value.GetL());
	EXPECT_EQ(123, value.GetCL());
	EXPECT_EQ(123, value.GetVL());
	EXPECT_EQ(123, value.GetCVL());
	EXPECT_EQ(123, value.GetR());
	EXPECT_EQ(123, value.GetCR());
	EXPECT_EQ(123, value.GetVR());
	EXPECT_EQ(123, value.GetCVR());
	value.~TestType();

	for (const UChar c : buffer)
		EXPECT_EQ(0xBB, c);

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
