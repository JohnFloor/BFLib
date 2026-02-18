#include "BF/BasicAlgorithm.hpp"

#include "BF/TestUtils.hpp"


// === Contains() ======================================================================================================


namespace {


struct ValueArchetype {
	constexpr bool operator==(const ValueArchetype& rightOp) const { return value == rightOp.value; }
	constexpr bool operator!=(const ValueArchetype&) const = delete;

	int value;
};


}	// namespace


BF_COMPILE_TIME_TEST()
{
	using List = std::initializer_list<ValueArchetype>;
	constexpr ValueArchetype a(1), b(2), c(3);

	static_assert(!BF::Contains(List{},           a));
	static_assert( BF::Contains(List{ a },        a));
	static_assert( BF::Contains(List{ a, b },     a));
	static_assert( BF::Contains(List{ a, b , c }, a));

	static_assert(!BF::Contains(List{},           b));
	static_assert(!BF::Contains(List{ a },        b));
	static_assert( BF::Contains(List{ a, b },     b));
	static_assert( BF::Contains(List{ a, b , c }, b));

	static_assert(!BF::Contains(List{},           c));
	static_assert(!BF::Contains(List{ a },        c));
	static_assert(!BF::Contains(List{ a, b },     c));
	static_assert( BF::Contains(List{ a, b , c }, c));
}
