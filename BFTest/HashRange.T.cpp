#include "BF/HashRange.hpp"

#include <list>
#include <string_view>
#include <vector>
#include "BF/TestUtils.hpp"


namespace {


// === Demo: Standard Library containers/views and cv element types ====================================================

// std::hash<const int>      BF_DUMMY;				// error: deleted ctor.
// std::hash<volatile int>   BF_DUMMY;				// error: deleted ctor.
// std::vector<const int>    BF_DUMMY;				// error: allocator is complaining
// std::vector<volatile int> BF_DUMMY;				// error: allocator is complaining
std::basic_string_view<const char>    BF_DUMMY;		// OK
std::basic_string_view<volatile char> BF_DUMMY;		// OK


// === Usage example ===================================================================================================

struct ComplexClass {
	class UniqueObjReps {
		Int64 a;
		Int32 b[2];
	};

	BF::Hash BF_GetHash() const {
		return { mInt,
				 std::string_view(mStr),
				 BF::HashRange(mRange),
				 BF::HashRawMemory(mUniqueObjReps),
				 BF::HashRawMemory(mMem1Begin, mMem1Size),
				 BF::HashRawMemory(mMem2Begin, mMem2End) };
	}

	UInt64              mInt;
	const char*         mStr;
	std::vector<UInt16> mRange;
	UniqueObjReps       mUniqueObjReps;
	std::byte*          mMem1Begin;
	std::size_t         mMem1Size;
	std::byte*          mMem2Begin;
	std::byte*          mMem2End;
};


// === class HashRange =================================================================================================

enum HashChoice {
	DoHash,
	DontHash
};


template <class Type, HashChoice HCh = DontHash>
void TestHashRange()
{
	Type value{};
	BF::HashRange h(value);

	if constexpr (HCh == DoHash)
		(void) std::hash<decltype(h)>()(h);
}


BF_COMPILE_TIME_TEST()
{
	TestHashRange<int[7],                             DoHash>();
	TestHashRange<const int[7],                       DoHash>();
	TestHashRange<std::vector<int>,                   DoHash>();
	TestHashRange<const std::vector<int>,             DoHash>();
	TestHashRange<std::basic_string_view<char>,       DoHash>();
	TestHashRange<std::basic_string_view<const char>, DoHash>();

	static_assert(std::is_final_v<BF::HashRange<int[7]>>);
	static_assert(std::is_final_v<BF::HashRange<std::vector<int>>>);

//	TestHashRange<int>();									// [CompilationError]: 'Range' must be a random access range.
//	TestHashRange<const int>();								// [CompilationError]: 'Range' must be a random access range.
//	TestHashRange<volatile int>();							// [CompilationError]: 'Range' must be a random access range.
//	TestHashRange<std::list<int>>();						// [CompilationError]: 'Range' must be a random access range.
//	TestHashRange<volatile std::vector<int>>();				// [CompilationError]: 'Range' must be a decayed type or a C array.
//	TestHashRange<volatile int[7]>();						// [CompilationError]: 'Range' element type must be a (possibly const) decayed type.
//	TestHashRange<int[7][77]>();							// [CompilationError]: 'Range' element type must be a (possibly const) decayed type.
//	TestHashRange<std::basic_string_view<volatile char>>();	// [CompilationError]: 'Range' element type must be a (possibly const) decayed type.
}


// === class HashRawMemory =============================================================================================

template <class Type>
void TestHashRawMemory()
{
	Type value{};
	BF::HashRawMemory h(value);
	(void) std::hash<decltype(h)>()(h);
}


BF_COMPILE_TIME_TEST()
{
	TestHashRawMemory<int>();
	TestHashRawMemory<const int>();
	TestHashRawMemory<int[7]>();
	TestHashRawMemory<const int[7]>();

	// BF::AsByteArray()'s static_assert's are tested in BF::AsByteArray()'s test

	static_assert(std::is_final_v<BF::HashRawMemory<7>>);
	static_assert(std::is_final_v<BF::HashRawMemory<std::dynamic_extent>>);

	struct Copy {
		Copy(const Copy&) {}
		char member = 0;
	};

	struct Empty {};

	using VolInt = volatile int;

//	{ int value = 0; BF::HashRawMemory<std::dynamic_extent> h(value); }		// [CompilationError]: Use a static extent.
//	{ int value = 0; BF::HashRawMemory<3>                   h(value); }		// [CompilationError]: 'Type' has bad size.
//	{ int*    b{}; VolInt*    e{}; BF::HashRawMemory h(b, e); }				// [CompilationError]: 'Type1' and 'Type2' must be the same.
//	{ VolInt* b{}; VolInt*    e{}; BF::HashRawMemory h(b, e); }				// [CompilationError]: 'Type1' must be decayed.
	{ int*    b{}; const int* e{}; BF::HashRawMemory h(b, e); }
//	{ Copy*   b{}; Copy*      e{}; BF::HashRawMemory h(b, e); }				// [CompilationError]: 'Type1' must be trivially copyable.
//	{ Empty*  b{}; Empty*     e{}; BF::HashRawMemory h(b, e); }				// [CompilationError]: A value of 'Type1' can be represented by two distinct bit patterns.
}


}	// namespace
