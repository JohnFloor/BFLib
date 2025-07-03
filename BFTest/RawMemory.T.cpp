#include "BF/RawMemory.hpp"

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
