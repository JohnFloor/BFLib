#include "BF/Compare.hpp"

#include "BF/TestUtils.hpp"


// === StdOrdering =====================================================================================================

template <class Type, bool ExpectedResult>
static void TestStdOrdering()
{
	static_assert(BF::IsStdOrdering<Type>         == ExpectedResult);
	static_assert(BF::IsStdOrderingT<Type>::value == ExpectedResult);
	static_assert(BF::StdOrdering<Type>           == ExpectedResult);
}


BF_COMPILE_TIME_TEST(BF::StdOrdering auto) {}


BF_COMPILE_TIME_TEST()
{
	TestStdOrdering<void,                  false>();
	TestStdOrdering<bool,                  false>();
	TestStdOrdering<int,                   false>();
	TestStdOrdering<std::strong_ordering,  true >();
	TestStdOrdering<std::weak_ordering,    true >();
	TestStdOrdering<std::partial_ordering, true >();
}
