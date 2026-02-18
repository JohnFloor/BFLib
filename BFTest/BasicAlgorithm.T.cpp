#include "BF/BasicAlgorithm.hpp"

#include "BF/TestUtils.hpp"


// === Contains() ======================================================================================================


namespace {


struct ValueArchetype1 {
	constexpr bool operator==(const ValueArchetype1& rightOp) const { return value == rightOp.value; }
	constexpr bool operator!=(const auto&) const = delete;

	int value;
};


struct ValueArchetype2 {
	constexpr bool operator==(int rightOp) const { return value == rightOp; }
	constexpr bool operator!=(const auto&) const = delete;

	int value;
};


}	// namespace


BF_COMPILE_TIME_TEST()
{
	using List = std::initializer_list<ValueArchetype1>;
	constexpr ValueArchetype1 a(1), b(2), c(3);

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


BF_COMPILE_TIME_TEST()
{
	using List = std::initializer_list<ValueArchetype2>;
	constexpr ValueArchetype2 a(1), b(2), c(3);

	static_assert(!BF::Contains(List{},           a.value));
	static_assert( BF::Contains(List{ a },        a.value));
	static_assert( BF::Contains(List{ a, b },     a.value));
	static_assert( BF::Contains(List{ a, b , c }, a.value));

	static_assert(!BF::Contains(List{},           b.value));
	static_assert(!BF::Contains(List{ a },        b.value));
	static_assert( BF::Contains(List{ a, b },     b.value));
	static_assert( BF::Contains(List{ a, b , c }, b.value));

	static_assert(!BF::Contains(List{},           c.value));
	static_assert(!BF::Contains(List{ a },        c.value));
	static_assert(!BF::Contains(List{ a, b },     c.value));
	static_assert( BF::Contains(List{ a, b , c }, c.value));
}
