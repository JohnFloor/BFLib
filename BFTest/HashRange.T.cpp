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
	class Inner {
		Int64 a;
		Int32 b[2];
	};

	BF::Hash BF_GetHash() const {
		return { mInt, std::string_view(mStr), BF::HashRange(mRange), BF::HashRawMemory(mMemory) };
	}

	UInt64              mInt;
	const char*         mStr;
	std::vector<UInt16> mRange;
	Inner               mMemory;
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
}


}	// namespace
