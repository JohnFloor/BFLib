#include "BF/TestUtils.hpp"

#include "gtest/gtest.h"
#include "BF/Duration.hpp"


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

	// TODO-CompilerBug: Currently you cannot put these directly into the static_assert. (Visual Studio v17.10.1, cl.exe 19.40.33811)
	static constexpr bool B1 = std::is_same_v<decltype(BF::LVal(x)),            Type&>;
	static constexpr bool B2 = std::is_same_v<decltype(BF::LVal(std::move(x))), Type&>;
	static_assert(B1);
	static_assert(B2);

	static constexpr bool B3 = &BF::LVal(x)            == &x;
	static constexpr bool B4 = &BF::LVal(std::move(x)) == &x;
	static_assert(B3);
	static_assert(B4);

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


// === SecureMemset ====================================================================================================

TEST(TestUtils, SecureMemset)
{
	char oneChar = 123;
	BF::SecureMemset(&oneChar, 124, 0);
	EXPECT_EQ(123, oneChar);

	char buffer[4] = {};
	BF::SecureMemset(buffer, 0xC5, sizeof(buffer));
	for (const UChar c : buffer)
		EXPECT_EQ(0xC5, c);

	const double duration = BF::MeasureDuration([] {
		char buffer[1024];
		for (size_t i = 0; i < MaxInt16 / 2; i++)
			BF::SecureMemset(buffer, 0, sizeof(buffer));
	});

	EXPECT_TRUE(duration / BF::GetNothingDuration() > 100);
}


// === MoveOnlyClass ===================================================================================================

static_assert( std::is_trivially_default_constructible_v<BF::MoveOnlyClass>);

static_assert(!std::is_constructible_v<BF::MoveOnlyClass, BF::MoveOnlyClass&>);
static_assert(!std::is_constructible_v<BF::MoveOnlyClass, const BF::MoveOnlyClass&>);
static_assert( std::is_trivially_constructible_v<BF::MoveOnlyClass, BF::MoveOnlyClass&&>);
static_assert(!std::is_constructible_v<BF::MoveOnlyClass, const BF::MoveOnlyClass&&>);

static_assert(!std::is_assignable_v<BF::MoveOnlyClass&, BF::MoveOnlyClass&>);
static_assert(!std::is_assignable_v<BF::MoveOnlyClass&, const BF::MoveOnlyClass&>);
static_assert( std::is_trivially_assignable_v<BF::MoveOnlyClass&, BF::MoveOnlyClass&&>);
static_assert(!std::is_assignable_v<BF::MoveOnlyClass&, const BF::MoveOnlyClass&&>);

static_assert( std::is_trivially_destructible_v<BF::MoveOnlyClass>);


// === ImmobileClass ===================================================================================================

static_assert( std::is_trivially_default_constructible_v<BF::ImmobileClass>);

static_assert(!std::is_constructible_v<BF::ImmobileClass, BF::ImmobileClass&>);
static_assert(!std::is_constructible_v<BF::ImmobileClass, const BF::ImmobileClass&>);
static_assert(!std::is_constructible_v<BF::ImmobileClass, BF::ImmobileClass&&>);
static_assert(!std::is_constructible_v<BF::ImmobileClass, const BF::ImmobileClass&&>);

static_assert(!std::is_assignable_v<BF::ImmobileClass&, BF::ImmobileClass&>);
static_assert(!std::is_assignable_v<BF::ImmobileClass&, const BF::ImmobileClass&>);
static_assert(!std::is_assignable_v<BF::ImmobileClass&, BF::ImmobileClass&&>);
static_assert(!std::is_assignable_v<BF::ImmobileClass&, const BF::ImmobileClass&&>);

static_assert( std::is_trivially_destructible_v<BF::ImmobileClass>);


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
