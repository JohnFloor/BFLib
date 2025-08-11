#include "BF/RawMemory.hpp"

#include <ranges>
#include "gtest/gtest.h"
#include "BF/Duration.hpp"
#include "BF/TestUtils.hpp"


// === GenPtr ==========================================================================================================

template <class Type>
static void TestGenPtr()
{
	static_assert(!std::is_reference_v<Type>);

	Type* const testPtr = (Type*)0x1122334455667788;
	BF::GenPtr  genPtr;

	genPtr = testPtr;

	EXPECT_EQ(testPtr, genPtr.AsPtr<Type>());
	EXPECT_EQ(testPtr, &BF::LVal(genPtr.AsRef<Type>()));
	EXPECT_EQ(testPtr, &BF::LVal(genPtr.AsRef<Type&>()));
	EXPECT_EQ(testPtr, &BF::LVal(genPtr.AsRef<Type&&>()));

	genPtr = (Type*)nullptr;

	EXPECT_EQ(nullptr, genPtr.AsPtr<Type>());
	EXPECT_TRUE(BF::IsNullReference(genPtr.AsRef<Type>()));
	EXPECT_TRUE(BF::IsNullReference(genPtr.AsRef<Type&>()));
	EXPECT_TRUE(BF::IsNullReference(genPtr.AsRef<Type&&>()));

	static_assert(std::is_same_v<decltype(genPtr.AsPtr<Type>()),   Type*>);
	static_assert(std::is_same_v<decltype(genPtr.AsRef<Type>()),   Type&&>);
	static_assert(std::is_same_v<decltype(genPtr.AsRef<Type&>()),  Type&>);
	static_assert(std::is_same_v<decltype(genPtr.AsRef<Type&&>()), Type&&>);

	static_assert(noexcept(genPtr = testPtr));
	static_assert(noexcept(genPtr.AsPtr<Type>()));
	static_assert(noexcept(genPtr.AsRef<Type>()));
	static_assert(noexcept(genPtr.AsRef<Type&>()));
	static_assert(noexcept(genPtr.AsRef<Type&&>()));
}


template <class Type>
static void TestGenPtrAllCV()
{
	TestGenPtr<Type>();
	TestGenPtr<const Type>();
	TestGenPtr<volatile Type>();
	TestGenPtr<const volatile Type>();
}


TEST(RawMemory, GenPtr)
{
	BF::GenPtr genPtr;
	genPtr = BF::Bad;
	EXPECT_EQ(BF::BadValuePtr, (UIntPtr&)genPtr);

	enum E;
	enum class EC;
	class C;
	union U;

	TestGenPtrAllCV<std::nullptr_t  >();
	TestGenPtrAllCV<int             >();
	TestGenPtrAllCV<void*           >();
	TestGenPtrAllCV<const void*     >();
	TestGenPtrAllCV<int C::*        >();
	TestGenPtrAllCV<int C::**       >();
	TestGenPtrAllCV<void (C::*)()   >();
	TestGenPtrAllCV<void (C::**)()  >();
	TestGenPtrAllCV<void (C::*)() & >();
	TestGenPtrAllCV<void (C::**)() &>();
	TestGenPtrAllCV<char[]          >();
	TestGenPtrAllCV<char(*)[]       >();
	TestGenPtrAllCV<char[4]         >();
	TestGenPtrAllCV<char(*)[4]      >();
	TestGenPtrAllCV<void ()         >();
	TestGenPtrAllCV<void (...)      >();
	TestGenPtrAllCV<void () noexcept>();
	TestGenPtrAllCV<void (*)()      >();
	TestGenPtrAllCV<E               >();
	TestGenPtrAllCV<EC              >();
	TestGenPtrAllCV<C               >();
	TestGenPtrAllCV<U               >();

	static_assert(std::is_trivial_v<BF::GenPtr>);
}


// === RawRange ========================================================================================================

template <std::size_t Size, std::size_t Align>
constexpr bool RawRangeExists = requires { typename BF::RawRange<Size, Align>; };

static_assert( RawRangeExists<8, 2>);
static_assert(!RawRangeExists<0, 1>);
static_assert(!RawRangeExists<9, 3>);
static_assert(!RawRangeExists<9, 2>);


template <std::size_t Size, std::size_t Align, UInt8 ExpectedArraySize, UInt8 ExpectedIntSize>
void TestRawRange()
{
	static_assert(1 <= Size && Size <= CHAR_MAX);
	static_assert(BF::IsPowerOf2(Align));
	static_assert(Size % Align == 0);
	static_assert(ExpectedIntSize * ExpectedArraySize == Size);

	using RR  = BF::RawRange<Size, Align>;
	using Int = std::remove_pointer_t<decltype(std::declval<RR>().begin())>;

	struct TestClass {
		alignas(Align) char array[Size] = {};
	};

	static_assert(sizeof(TestClass)  == Size);
	static_assert(alignof(TestClass) == Align);

	TestClass        testObj;
	const TestClass& cTestObj = testObj;

	static_assert(std::is_same_v<decltype(BF::AsRawRange(testObj)),        RR&>);
	static_assert(std::is_same_v<decltype(BF::AsRawRange(cTestObj)), const RR&>);

	RR&       rr           = BF::AsRawRange(testObj);
	const RR& crr          = BF::AsRawRange(cTestObj);
	auto&     testObjArray = reinterpret_cast<Int(&)[ExpectedArraySize]>(testObj);

	static_assert( std::is_final_v<RR>);
	static_assert( std::is_base_of_v<BF::ImmobileClass, RR>);
	static_assert(!std::is_default_constructible_v<RR>);
	static_assert(!std::is_constructible_v<RR, TestClass&>);
	static_assert(!std::is_constructible_v<RR, TestClass&&>);
	static_assert( std::ranges::random_access_range<RR>);

	static_assert(sizeof(RR)  == Size);
	static_assert(alignof(RR) == Align);
	static_assert(RR::GetSize() == ExpectedArraySize);
	static_assert(BF::IsDecayed<Int> && BF::IsUInteger<Int> && sizeof(Int) == ExpectedIntSize);

	static_assert(std::is_same_v<decltype(rr[0]),      Int&>);
	static_assert(std::is_same_v<decltype(rr.begin()), Int*>);
	static_assert(std::is_same_v<decltype(rr.end()),   Int*>);
	EXPECT_EQ(rr.end() - rr.begin(), ExpectedArraySize);

	static_assert(std::is_same_v<decltype(crr[0]),      const Int&>);
	static_assert(std::is_same_v<decltype(crr.begin()), const Int*>);
	static_assert(std::is_same_v<decltype(crr.end()),   const Int*>);
	EXPECT_EQ(crr.end() - crr.begin(), ExpectedArraySize);

	UInt8 i;

	i = 0;
	for (Int& item : rr)
		EXPECT_EQ(&item, &testObjArray[i++]);

	i = 0;
	for (const Int& item : crr)
		EXPECT_EQ(&item, &testObjArray[i++]);

	for (UInt8 i = 0; i < ExpectedArraySize; i++) {
		EXPECT_EQ(&rr[i],  &testObjArray[i]);
		EXPECT_EQ(&crr[i], &testObjArray[i]);
	}
}


TEST(RawMemory, RawRange)
{
	TestRawRange< 8,  1,  8,  1>();
	TestRawRange< 8,  2,  4,  2>();
	TestRawRange< 8,  4,  2,  4>();
	TestRawRange< 8,  8,  1,  8>();

	TestRawRange<32,  1, 32,  1>();
	TestRawRange<32,  2, 16,  2>();
	TestRawRange<32,  4,  8,  4>();
	TestRawRange<32,  8,  4,  8>();

	TestRawRange<32, 16,  4,  8>();
	TestRawRange<32, 32,  4,  8>();
	TestRawRange<64, 16,  8,  8>();
	TestRawRange<64, 32,  8,  8>();

	const int arr[7]{};
	BF::AsRawRange(arr);

//	BF::RawRange BF_DUMMY;							// [CompilationError]: attempting to reference a deleted function
//	BF::RawRange BF_DUMMY(123);						// [CompilationError]: attempting to reference a deleted function

	struct Virt {
		virtual ~Virt() = default;
	};

	struct Padded {
		UInt16 a = 0;
		char   b = 0;
	};

	struct Empty {};

//	{ volatile int x = 0;    BF::AsRawRange(x); }	// [CompilationError]: 'Type' must be decayed/const, or a bounded array of decayed/const types.
//	{ volatile int x[7]{};   BF::AsRawRange(x); }	// [CompilationError]: 'Type' must be decayed/const, or a bounded array of decayed/const types.
//	{ Virt         x;        BF::AsRawRange(x); }	// [CompilationError]: 'Type' must be trivially copyable.
//	{ float        x = 0.0;  BF::AsRawRange(x); }	// [CompilationError]: A value of 'Type' can be represented by two distinct bit patterns.
//	{ double       x = 0.0;  BF::AsRawRange(x); }	// [CompilationError]: A value of 'Type' can be represented by two distinct bit patterns.
//	{ Padded       x;        BF::AsRawRange(x); }	// [CompilationError]: A value of 'Type' can be represented by two distinct bit patterns.
//	{ Empty        x;        BF::AsRawRange(x); }	// [CompilationError]: A value of 'Type' can be represented by two distinct bit patterns.
}


// === IsNullReference =================================================================================================

TEST(RawMemory, IsNullReference)
{
	int*         null = nullptr;
	volatile int vi   = 0;

	EXPECT_TRUE (BF::IsNullReference(*null));
	EXPECT_FALSE(BF::IsNullReference(vi));
	EXPECT_FALSE(BF::IsNullReference(std::move(vi)));		// would not compile, if parameter would be 'const auto&'
	EXPECT_FALSE(BF::IsNullReference(123));

	static_assert(noexcept(BF::IsNullReference(0)));
}


// === SecureMemset ====================================================================================================

TEST(RawMemory, SecureMemset)
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
