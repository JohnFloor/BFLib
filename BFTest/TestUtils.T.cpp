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


// === AssertTrivialCopyMoveDtor() =====================================================================================

BF_COMPILE_TIME_TEST()
{
	struct T0 {
		T0(const T0&) = default;
		T0(T0&&) = default;
		T0& operator=(const T0&) = default;
		T0& operator=(T0&&) = default;
		~T0() = default;
	};

	BF::AssertTrivialCopyMoveDtor<T0>();
}


BF_COMPILE_TIME_TEST()
{
	struct T1 {
		T1(const T1&) {}
		T1(T1&&) = default;
		T1& operator=(const T1&) = default;
		T1& operator=(T1&&) = default;
		~T1() = default;
	};

//	BF::AssertTrivialCopyMoveDtor<T1>();		// [CompilationError]: static assertion failed: 'std::is_trivially_copyable_v<Type>'

	struct T2 {
		T2(const T2&) = default;
		T2(T2&&) noexcept {}
		T2& operator=(const T2&) = default;
		T2& operator=(T2&&) = default;
		~T2() = default;
	};

//	BF::AssertTrivialCopyMoveDtor<T2>();		// [CompilationError]: static assertion failed: 'std::is_trivially_copyable_v<Type>'

	struct T3 {
		T3(const T3&) = default;
		T3(T3&&) = default;
		T3& operator=(const T3&) { return *this; }
		T3& operator=(T3&&) = default;
		~T3() = default;
	};

//	BF::AssertTrivialCopyMoveDtor<T3>();		// [CompilationError]: static assertion failed: 'std::is_trivially_copyable_v<Type>'

	struct T4 {
		T4(const T4&) = default;
		T4(T4&&) = default;
		T4& operator=(const T4&) = default;
		T4& operator=(T4&&) noexcept { return *this; }
		~T4() = default;
	};

//	BF::AssertTrivialCopyMoveDtor<T4>();		// [CompilationError]: static assertion failed: 'std::is_trivially_copyable_v<Type>'

	struct T5 {
		T5(const T5&) = default;
		T5(T5&&) = default;
		T5& operator=(const T5&) = default;
		T5& operator=(T5&&) = default;
		~T5() {}
	};

//	BF::AssertTrivialCopyMoveDtor<T5>();		// [CompilationError]: static assertion failed: 'std::is_trivially_copyable_v<Type>'

	struct T5v {
		T5v(const T5v&) = default;
		T5v(T5v&&) = default;
		T5v& operator=(const T5v&) = default;
		T5v& operator=(T5v&&) = default;
		virtual ~T5v() = default;
	};

//	BF::AssertTrivialCopyMoveDtor<T5v>();		// [CompilationError]: static assertion failed: 'std::is_trivially_copyable_v<Type>'
}


#define DEFINE_CLASS_WITH_EXTRA_CTOR(Class, ctorParam)	\
	struct Class {										\
		Class(const Class&) = default;					\
		Class(Class&&) = default;						\
		Class& operator=(const Class&) = default;		\
		Class& operator=(Class&&) = default;			\
		~Class() = default;								\
														\
		Class(ctorParam) {}								\
	};


#define DEFINE_CLASS_WITH_EXTRA_ASS(Class, assParam)	\
	struct Class {										\
		Class(const Class&) = default;					\
		Class(Class&&) = default;						\
		Class& operator=(const Class&) = default;		\
		Class& operator=(Class&&) = default;			\
		~Class() = default;								\
														\
		Class& operator=(assParam) { return *this; }	\
	};


namespace {
	DEFINE_CLASS_WITH_EXTRA_CTOR(CtorR,   auto&)
	DEFINE_CLASS_WITH_EXTRA_CTOR(CtorCR,  const auto&)
	DEFINE_CLASS_WITH_EXTRA_CTOR(CtorRR,  auto&&)			// forwarding
	DEFINE_CLASS_WITH_EXTRA_CTOR(CtorCRR, const auto&&)

	DEFINE_CLASS_WITH_EXTRA_ASS (AssR,    auto&)
	DEFINE_CLASS_WITH_EXTRA_ASS (AssCR,   const auto&)
	DEFINE_CLASS_WITH_EXTRA_ASS (AssRR,   auto&&)			// forwarding
	DEFINE_CLASS_WITH_EXTRA_ASS (AssCRR,  const auto&&)
}	// namespace


BF_COMPILE_TIME_TEST()
{
//	BF::AssertTrivialCopyMoveDtor<CtorR>();		// [CompilationError]: static assertion failed: 'std::is_trivially_constructible_v<Type, Type&>'
	BF::AssertTrivialCopyMoveDtor<CtorCR>();
//	BF::AssertTrivialCopyMoveDtor<CtorRR>();	// [CompilationError]: static assertion failed: 'std::is_trivially_constructible_v<Type, Type&>'
//	BF::AssertTrivialCopyMoveDtor<CtorCRR>();	// [CompilationError]: static assertion failed: 'std::is_trivially_constructible_v<Type, const Type&&>'

//	BF::AssertTrivialCopyMoveDtor<AssR>();		// [CompilationError]: static assertion failed: 'std::is_trivially_assignable_v<Type&, Type&>'
	BF::AssertTrivialCopyMoveDtor<AssCR>();
//	BF::AssertTrivialCopyMoveDtor<AssRR>();		// [CompilationError]: static assertion failed: 'std::is_trivially_assignable_v<Type&, Type&>'
//	BF::AssertTrivialCopyMoveDtor<AssCRR>();	// [CompilationError]: static assertion failed: 'std::is_trivially_assignable_v<Type&, const Type&&>'
}
