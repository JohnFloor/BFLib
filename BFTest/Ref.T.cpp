#include "BF/Ref.hpp"

#include <concepts>
#include "gtest/gtest.h"
#include "BF/TestUtils.hpp"
#include "GTU/Diary.hpp"


namespace {


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////// Helpers /////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


template <class Source, class Target>
void TestNotSelf()
{
	GTU_XD("C") { Source       s; Target    t = s;           }
	GTU_XD("M") { Source       s; Target    t = BF::Move(s); }
	GTU_XD("C") { const Source s; Target    t = s;           }
	GTU_XD("C") { const Source s; Target    t = BF::Move(s); }

	GTU_XD("C") { Source       s; Target t; t = s;           }
	GTU_XD("M") { Source       s; Target t; t = BF::Move(s); }
	GTU_XD("C") { const Source s; Target t; t = s;           }
	GTU_XD("C") { const Source s; Target t; t = BF::Move(s); }
}


template <class Type, class ToType>
void TestConstCast()
{
	BF::Ref<Type> r;
	const          BF::Ref<Type> cr;
	volatile       BF::Ref<Type> vr;
	const volatile BF::Ref<Type> cvr;

	static_assert(std::is_same_v<decltype(r.ConstCast<ToType>()),                  BF::Ref<ToType>&>);
	static_assert(std::is_same_v<decltype(cr.ConstCast<ToType>()),  const          BF::Ref<ToType>&>);
	static_assert(std::is_same_v<decltype(vr.ConstCast<ToType>()),  volatile       BF::Ref<ToType>&>);
	static_assert(std::is_same_v<decltype(cvr.ConstCast<ToType>()), const volatile BF::Ref<ToType>&>);

	static_assert(std::is_same_v<decltype(BF::Move(r).ConstCast<ToType>()),                  BF::Ref<ToType>&&>);
	static_assert(std::is_same_v<decltype(BF::Move(cr).ConstCast<ToType>()),  const          BF::Ref<ToType>&&>);
	static_assert(std::is_same_v<decltype(BF::Move(vr).ConstCast<ToType>()),  volatile       BF::Ref<ToType>&&>);
	static_assert(std::is_same_v<decltype(BF::Move(cvr).ConstCast<ToType>()), const volatile BF::Ref<ToType>&&>);

	EXPECT_EQ((void*) &r.ConstCast<ToType>(),   &r);
	EXPECT_EQ((void*) &cr.ConstCast<ToType>(),  &cr);
	EXPECT_EQ((void*) &vr.ConstCast<ToType>(),  &vr);
	EXPECT_EQ((void*) &cvr.ConstCast<ToType>(), &cvr);

	EXPECT_EQ((void*) &BF::LVal(BF::Move(r).ConstCast<ToType>()),   &r);
	EXPECT_EQ((void*) &BF::LVal(BF::Move(cr).ConstCast<ToType>()),  &cr);
	EXPECT_EQ((void*) &BF::LVal(BF::Move(vr).ConstCast<ToType>()),  &vr);
	EXPECT_EQ((void*) &BF::LVal(BF::Move(cvr).ConstCast<ToType>()), &cvr);
}


constexpr int UnorderedInt = -1;


std::partial_ordering PartiallyOrder(int leftOp, int rightOp)
{
	if (leftOp == UnorderedInt || rightOp == UnorderedInt)
		return std::partial_ordering::unordered;
	else
		return leftOp <=> rightOp;
}


enum class ComparisonType {
	Eq  = 0b0000001,	// ==
	Ne  = 0b0000010,	// !=
	Lt  = 0b0000100,	// <
	Gt  = 0b0001000,	// >
	Le  = 0b0010000,	// <=
	Ge  = 0b0100000,	// >=
	W3  = 0b1000000,	// <=>

	Equality   = Eq | Ne,
	Relational = Lt | Gt | Le | Ge
};


constexpr ComparisonType operator|(ComparisonType leftOp, ComparisonType rightOp)
{
	using U = std::underlying_type_t<ComparisonType>;
	return ComparisonType(static_cast<U>(leftOp) | static_cast<U>(rightOp));
}


constexpr bool Has(ComparisonType flags, ComparisonType value)
{
	using U = std::underlying_type_t<ComparisonType>;
	return static_cast<U>(flags) & static_cast<U>(value);
}


template <ComparisonType CT, class Type>
void CheckComparisonOp1(const Type& t1, const Type& t2)
{
	const BF::Ref<Type> rt1 = t1;
	const BF::Ref<Type> rt2 = t2;

	if constexpr (Has(CT, ComparisonType::Eq)) EXPECT_EQ(rt1 == rt2, t1 == t2);
	if constexpr (Has(CT, ComparisonType::Ne)) EXPECT_EQ(rt1 != rt2, t1 != t2);
	if constexpr (Has(CT, ComparisonType::Lt)) EXPECT_EQ(rt1 <  rt2, t1 <  t2);
	if constexpr (Has(CT, ComparisonType::Gt)) EXPECT_EQ(rt1 >  rt2, t1 >  t2);
	if constexpr (Has(CT, ComparisonType::Le)) EXPECT_EQ(rt1 <= rt2, t1 <= t2);
	if constexpr (Has(CT, ComparisonType::Ge)) EXPECT_EQ(rt1 >= rt2, t1 >= t2);

	if constexpr (Has(CT, ComparisonType::W3)) {
		static_assert(std::is_same_v<decltype(rt1 <=> rt2), decltype(t1 <=> t2)>);
		EXPECT_EQ(rt1 <=> rt2, t1 <=> t2);
	}
}


template <ComparisonType CT, class Type>
void CheckComparisonOp()
{
	const Type t1(1), t2(2), tU(UnorderedInt);

	CheckComparisonOp1<CT>(t1, t1);

	CheckComparisonOp1<CT>(t1, t2);
	CheckComparisonOp1<CT>(t2, t1);

	CheckComparisonOp1<CT>(t1, tU);
	CheckComparisonOp1<CT>(tU, t1);
}


namespace CompOpCheckers {

struct Eq {
	template <class T>
	static constexpr bool           Check = requires (const T x) { { x == x } -> std::convertible_to<bool>; };
	static constexpr ComparisonType CT    = ComparisonType::Equality;
};

struct Ne {
	template <class T>
	static constexpr bool           Check = requires (const T x) { { x != x } -> std::convertible_to<bool>; };
	static constexpr ComparisonType CT    = ComparisonType::Ne;
};

struct Lt {
	template <class T>
	static constexpr bool           Check = requires (const T x) { { x <  x } -> std::convertible_to<bool>; };
	static constexpr ComparisonType CT    = ComparisonType::Lt;
};

struct Gt {
	template <class T>
	static constexpr bool           Check = requires (const T x) { { x >  x } -> std::convertible_to<bool>; };
	static constexpr ComparisonType CT    = ComparisonType::Gt;
};

struct Le {
	template <class T>
	static constexpr bool           Check = requires (const T x) { { x <= x } -> std::convertible_to<bool>; };
	static constexpr ComparisonType CT    = ComparisonType::Le;
};

struct Ge {
	template <class T>
	static constexpr bool           Check = requires (const T x) { { x >= x } -> std::convertible_to<bool>; };
	static constexpr ComparisonType CT    = ComparisonType::Ge;
};

template <class Ordering>
struct WR {
	template <class T>
	static constexpr bool           Check = requires (const T x) { { x <=> x } -> std::convertible_to<Ordering>; };
	static constexpr ComparisonType CT    = ComparisonType::W3 | ComparisonType::Relational;
};

template <class Ordering>
struct W3 {
	template <class T>
	static constexpr bool           Check = std::three_way_comparable<T, Ordering>;
	static constexpr ComparisonType CT    = ComparisonType::W3 | ComparisonType::Relational | ComparisonType::Equality;
};

}	// namespace CompOpCheckers


template <std::constructible_from<int> Type, class CompOpChecker>
void AssertIsComparableAtTheSameTime()
{
	constexpr bool T = CompOpChecker::template Check<Type>;
	static_assert(T == CompOpChecker::template Check<BF::Ref<Type>>);
	static_assert(T == CompOpChecker::template Check<BF::Ref<volatile Type>>);

	if constexpr (T)
		CheckComparisonOp<CompOpChecker::CT, Type>();
}


template <std::constructible_from<int> Type>
void AssertIsComparableAtTheSameTime()
{
	AssertIsComparableAtTheSameTime<Type, CompOpCheckers::Eq>();
	AssertIsComparableAtTheSameTime<Type, CompOpCheckers::Ne>();
	AssertIsComparableAtTheSameTime<Type, CompOpCheckers::Lt>();
	AssertIsComparableAtTheSameTime<Type, CompOpCheckers::Gt>();
	AssertIsComparableAtTheSameTime<Type, CompOpCheckers::Le>();
	AssertIsComparableAtTheSameTime<Type, CompOpCheckers::Ge>();
	AssertIsComparableAtTheSameTime<Type, CompOpCheckers::WR<std::strong_ordering>>();
	AssertIsComparableAtTheSameTime<Type, CompOpCheckers::WR<std::weak_ordering>>();
	AssertIsComparableAtTheSameTime<Type, CompOpCheckers::WR<std::partial_ordering>>();
	AssertIsComparableAtTheSameTime<Type, CompOpCheckers::W3<std::strong_ordering>>();
	AssertIsComparableAtTheSameTime<Type, CompOpCheckers::W3<std::weak_ordering>>();
	AssertIsComparableAtTheSameTime<Type, CompOpCheckers::W3<std::partial_ordering>>();
}


template <class Type>
void AssertIsHashableAtTheSameTime()
{
	constexpr bool T = BF::StdHashable<Type>;
	static_assert(T == BF::StdHashable<BF::Ref<Type>>);
	static_assert(T == BF::StdHashable<BF::Ref<volatile Type>>);

	if constexpr (T) {
		const Type          t;
		const BF::Ref<Type> rt;
		EXPECT_EQ(std::hash<BF::Ref<Type>>()(rt), std::hash<Type>()(t));
	}
}


}	// namespace


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////// Test cases //////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


TEST(Ref, Basics)
{
	struct X {};
	struct Y : X {};
	struct I { I(int) {} };
	union U {};

	BF::AssertTrivialCopyMoveDtor<BF::Ref<X>>();

//	{ BF::Ref<void*> ref; }								// [CompilationError]: 'Type' should be a (possibly cv-qualified) class.
//	{ BF::Ref<X*> ref; }								// [CompilationError]: 'Type' should be a (possibly cv-qualified) class.
//	{ BF::Ref<X&> ref; }								// [CompilationError]: 'Type' should be a (possibly cv-qualified) class.
//	{ BF::Ref<U> ref; }									// [CompilationError]: 'Type' should be a (possibly cv-qualified) class.

//	{ BF::Ref<Y> y; BF::Ref<X>    x = y; }				// [CompilationError]: 'SourceType' should be the same as 'Type', ignoring cv-qualifiers.
//	{ BF::Ref<Y> y; BF::Ref<X> x; x = y; }				// [CompilationError]: 'SourceType' should be the same as 'Type', ignoring cv-qualifiers.
//	{ BF::Ref<Y> y; BF::Ref<X>    x = std::move(y); }	// [CompilationError]: 'SourceType' should be the same as 'Type', ignoring cv-qualifiers.
//	{ BF::Ref<Y> y; BF::Ref<X> x; x = std::move(y); }	// [CompilationError]: 'SourceType' should be the same as 'Type', ignoring cv-qualifiers.

//	{ BF::Ref<X> x; BF::Ref<Y>    y = x; }				// [CompilationError]: 'SourceType' should be the same as 'Type', ignoring cv-qualifiers.
//	{ BF::Ref<X> x; BF::Ref<Y> y; y = x; }				// [CompilationError]: 'SourceType' should be the same as 'Type', ignoring cv-qualifiers.
//	{ BF::Ref<X> x; BF::Ref<Y>    y = std::move(x); }	// [CompilationError]: 'SourceType' should be the same as 'Type', ignoring cv-qualifiers.
//	{ BF::Ref<X> x; BF::Ref<Y> y; y = std::move(x); }	// [CompilationError]: 'SourceType' should be the same as 'Type', ignoring cv-qualifiers.

	{ BF::Ref<X> defaultCtor; }
//	{ BF::Ref<I> defaultCtor; }							// [CompilationError]: no appropriate default constructor available
	{ BF::Ref<I> ctor = 123;  }
}


TEST(Ref, DefaultCtorDtor)
{
	struct Window {
		Window()	{ GTU::Push('+'); }
		~Window()	{ GTU::Push('-'); }
	};

	// Note: We know, that the from Pars... ctor. is called, because the from BF::Ref<SourceType> one cannot be called
	//       with 0 arguments.

	GTU_XD("+-") { BF::Ref<Window> ref; }
}


TEST(Ref, FromPars)
{
	class HWND {};

	struct Window {
		Window()							 {}
		Window(int)							 { GTU::Push('1'); }
		Window(int, int)					 { GTU::Push('2'); }

		Window(const HWND& hwnd)			 { GTU::Push('c'); }
		Window(HWND&& hwnd)					 { GTU::Push('m'); }
		Window& operator=(const HWND& hwnd)	 { GTU::Push('c'); return *this; }
		Window& operator=(HWND&& hwnd)		 { GTU::Push('m'); return *this; }

		Window(const Window&)				 { GTU::Push('C'); }
		Window(Window&&) noexcept			 { GTU::Push('M'); }
		Window& operator=(const Window&)	 { GTU::Push('C'); return *this; }
		Window& operator=(Window&&) noexcept { GTU::Push('M'); return *this; }
	};

	// Note: We know, that the from Pars... ctor./assignment is called, because for the from BF::Ref<SourceType> one
	//       a temporary BF::Ref<SourceType> would need to be constructed. We exclude this scenario by observing, that
	//       no extra 'M' event is logged.

	GTU_XD("1") { BF::Ref<Window> ref(1);    }
	GTU_XD("2") { BF::Ref<Window> ref(1, 2); }

	GTU_XD("c") { HWND h;   BF::Ref<Window>      ref = h;            }
	GTU_XD("c") { HWND h;   BF::Ref<Window> ref; ref = h;            }
	GTU_XD("m") { HWND h;   BF::Ref<Window>      ref = std::move(h); }
	GTU_XD("m") { HWND h;   BF::Ref<Window> ref; ref = std::move(h); }

	GTU_XD("C") { Window w; BF::Ref<Window>      ref = w;            }
	GTU_XD("C") { Window w; BF::Ref<Window> ref; ref = w;            }
	GTU_XD("M") { Window w; BF::Ref<Window>      ref = std::move(w); }
	GTU_XD("M") { Window w; BF::Ref<Window> ref; ref = std::move(w); }
}


TEST(Ref, CVCorrectness)
{
	struct Window {
		Window() = default;
		Window(const Window&)				 { GTU::Push('C'); }
		Window(Window&&) noexcept			 { GTU::Push('M'); }
		Window& operator=(const Window&)	 { GTU::Push('C'); return *this; }
		Window& operator=(Window&&) noexcept { GTU::Push('M'); return *this; }
	};

	// Note: We know, that the from BF::Ref<SourceType> ctor./assignment is called, because the from Pars... one would
	//       not compile (Window is not constructible/assignable from BF::Ref<SourceType>).

	using W   = BF::Ref<Window>;
	using CW  = BF::Ref<const Window>;
	using VW  = BF::Ref<volatile Window>;
	using CVW = BF::Ref<const volatile Window>;

		// Copy ctor.

	GTU_XD("C") { W   s; W   t = s; }
	GTU_XD("C") { W   s; CW  t = s; }
	GTU_XD("C") { W   s; VW  t = s; }
	GTU_XD("C") { W   s; CVW t = s; }

//	            { CW  s; W   t = s; }					// [CompilationError]: Conversion would lose a const qualifier.
	GTU_XD("C") { CW  s; CW  t = s; }
//	            { CW  s; VW  t = s; }					// [CompilationError]: Conversion would lose a const qualifier.
	GTU_XD("C") { CW  s; CVW t = s; }

//	            { VW  s; W   t = s; }					// [CompilationError]: Conversion would lose a volatile qualifier.
//	            { VW  s; CW  t = s; }					// [CompilationError]: Conversion would lose a volatile qualifier.
	GTU_XD("C") { VW  s; VW  t = s; }
	GTU_XD("C") { VW  s; CVW t = s; }

//	            { CVW s; W   t = s; }					// [CompilationError]: Conversion would lose a const qualifier.
//	            { CVW s; CW  t = s; }					// [CompilationError]: Conversion would lose a volatile qualifier.
//	            { CVW s; VW  t = s; }					// [CompilationError]: Conversion would lose a const qualifier.
	GTU_XD("C") { CVW s; CVW t = s; }

		// Move ctor.

	GTU_XD("M") { W   s; W   t = std::move(s); }
	GTU_XD("M") { W   s; CW  t = std::move(s); }
	GTU_XD("M") { W   s; VW  t = std::move(s); }
	GTU_XD("M") { W   s; CVW t = std::move(s); }

//	            { CW  s; W   t = std::move(s); }		// [CompilationError]: Conversion would lose a const qualifier.
	GTU_XD("M") { CW  s; CW  t = std::move(s); }
//	            { CW  s; VW  t = std::move(s); }		// [CompilationError]: Conversion would lose a const qualifier.
	GTU_XD("M") { CW  s; CVW t = std::move(s); }

//	            { VW  s; W   t = std::move(s); }		// [CompilationError]: Conversion would lose a volatile qualifier.
//	            { VW  s; CW  t = std::move(s); }		// [CompilationError]: Conversion would lose a volatile qualifier.
	GTU_XD("M") { VW  s; VW  t = std::move(s); }
	GTU_XD("M") { VW  s; CVW t = std::move(s); }

//	            { CVW s; W   t = std::move(s); }		// [CompilationError]: Conversion would lose a const qualifier.
//	            { CVW s; CW  t = std::move(s); }		// [CompilationError]: Conversion would lose a volatile qualifier.
//	            { CVW s; VW  t = std::move(s); }		// [CompilationError]: Conversion would lose a const qualifier.
	GTU_XD("M") { CVW s; CVW t = std::move(s); }

		// Copy assignment

	GTU_XD("C") { W   s; W   t; t = s; }
	GTU_XD("C") { W   s; CW  t; t = s; }
	GTU_XD("C") { W   s; VW  t; t = s; }
	GTU_XD("C") { W   s; CVW t; t = s; }

//	            { CW  s; W   t; t = s; }				// [CompilationError]: Conversion would lose a const qualifier.
	GTU_XD("C") { CW  s; CW  t; t = s; }
//	            { CW  s; VW  t; t = s; }				// [CompilationError]: Conversion would lose a const qualifier.
	GTU_XD("C") { CW  s; CVW t; t = s; }

//	            { VW  s; W   t; t = s; }				// [CompilationError]: Conversion would lose a volatile qualifier.
//	            { VW  s; CW  t; t = s; }				// [CompilationError]: Conversion would lose a volatile qualifier.
	GTU_XD("C") { VW  s; VW  t; t = s; }
	GTU_XD("C") { VW  s; CVW t; t = s; }

//	            { CVW s; W   t; t = s; }				// [CompilationError]: Conversion would lose a const qualifier.
//	            { CVW s; CW  t; t = s; }				// [CompilationError]: Conversion would lose a volatile qualifier.
//	            { CVW s; VW  t; t = s; }				// [CompilationError]: Conversion would lose a const qualifier.
	GTU_XD("C") { CVW s; CVW t; t = s; }

		// Move assignment

	GTU_XD("M") { W   s; W   t; t = std::move(s); }
	GTU_XD("M") { W   s; CW  t; t = std::move(s); }
	GTU_XD("M") { W   s; VW  t; t = std::move(s); }
	GTU_XD("M") { W   s; CVW t; t = std::move(s); }

//	            { CW  s; W   t; t = std::move(s); }		// [CompilationError]: Conversion would lose a const qualifier.
	GTU_XD("M") { CW  s; CW  t; t = std::move(s); }
//	            { CW  s; VW  t; t = std::move(s); }		// [CompilationError]: Conversion would lose a const qualifier.
	GTU_XD("M") { CW  s; CVW t; t = std::move(s); }

//	            { VW  s; W   t; t = std::move(s); }		// [CompilationError]: Conversion would lose a volatile qualifier.
//	            { VW  s; CW  t; t = std::move(s); }		// [CompilationError]: Conversion would lose a volatile qualifier.
	GTU_XD("M") { VW  s; VW  t; t = std::move(s); }
	GTU_XD("M") { VW  s; CVW t; t = std::move(s); }

//	            { CVW s; W   t; t = std::move(s); }		// [CompilationError]: Conversion would lose a const qualifier.
//	            { CVW s; CW  t; t = std::move(s); }		// [CompilationError]: Conversion would lose a volatile qualifier.
//	            { CVW s; VW  t; t = std::move(s); }		// [CompilationError]: Conversion would lose a const qualifier.
	GTU_XD("M") { CVW s; CVW t; t = std::move(s); }
}


TEST(Ref, NotSelf)
{
	struct Window {
		Window() = default;
		Window(const Window&)				 { GTU::Push('C'); }
		Window(Window&&) noexcept			 { GTU::Push('M'); }
		Window& operator=(const Window&)	 { GTU::Push('C'); return *this; }
		Window& operator=(Window&&) noexcept { GTU::Push('M'); return *this; }
	};

	// Note: We know, that the from BF::Ref<SourceType> ctor./assignment is called, because the from Pars... one would
	//       not compile (Window is not constructible/assignable from BF::Ref<SourceType>).

	using W  = BF::Ref<Window>;
	using CW = BF::Ref<const Window>;
	struct DW : W {};

	TestNotSelf<W,  W >();
	TestNotSelf<DW, W >();
	TestNotSelf<W,  CW>();
	TestNotSelf<DW, CW>();
}


TEST(Ref, Dereferencing)
{
	class X {};

	const BF::Ref<X> r;
//	*r;													// [CompilationError]: attempting to reference a deleted function
}


TEST(Ref, Indirection)
{
	struct Window {
		void Get() const	{}
		void Set()			{}
	};

	using W  = BF::Ref<Window>;
	using CW = BF::Ref<const Window>;

	{ W        r; r->Get(); }
	{ CW       r; r->Get(); }
	{ const W  r; r->Get(); }
	{ const CW r; r->Get(); }

	{ W        r; r->Set(); }
//	{ CW       r; r->Set(); }							// [CompilationError]: Set(void)': cannot convert 'this' pointer
	{ const W  r; r->Set(); }
//	{ const CW r; r->Set(); }							// [CompilationError]: Set(void)': cannot convert 'this' pointer
}


TEST(Ref, ConstCast)
{
	struct X {};
	struct Y : X {};
	union U {};

//	BF::Ref<X>().ConstCast<void*>();					// [CompilationError]: 'ToType' should be the same as 'Type', ignoring cv-qualifiers.
//	BF::Ref<X>().ConstCast<X*>();						// [CompilationError]: 'ToType' should be the same as 'Type', ignoring cv-qualifiers.
//	BF::Ref<X>().ConstCast<X&>();						// [CompilationError]: 'ToType' should be the same as 'Type', ignoring cv-qualifiers.
//	BF::Ref<X>().ConstCast<U>();						// [CompilationError]: 'ToType' should be the same as 'Type', ignoring cv-qualifiers.
//	BF::Ref<X>().ConstCast<Y>();						// [CompilationError]: 'ToType' should be the same as 'Type', ignoring cv-qualifiers.
//	BF::Ref<Y>().ConstCast<X>();						// [CompilationError]: 'ToType' should be the same as 'Type', ignoring cv-qualifiers.

	TestConstCast<X,                X>();
	TestConstCast<X, const          X>();
	TestConstCast<X, volatile       X>();
	TestConstCast<X, const volatile X>();

	TestConstCast<               X, X>();
	TestConstCast<const          X, X>();
	TestConstCast<volatile       X, X>();
	TestConstCast<const volatile X, X>();
}


TEST(Ref, Comparison)
{
	struct X  { int m; };

	struct Eq { bool operator==(const Eq& o) const { return m == o.m; } int m; };
	struct Ne { bool operator!=(const Ne& o) const { return m != o.m; } int m; };
	struct Lt { bool operator< (const Lt& o) const { return m <  o.m; } int m; };
	struct Gt { bool operator> (const Gt& o) const { return m >  o.m; } int m; };
	struct Le { bool operator<=(const Le& o) const { return m <= o.m; } int m; };
	struct Ge { bool operator>=(const Ge& o) const { return m >= o.m; } int m; };

	struct S  { std::strong_ordering  operator<=>(const S&) const = default;                            int m; };
	struct W  { std::weak_ordering    operator<=>(const W&) const = default;                            int m; };
	struct P  { std::partial_ordering operator<=>(const P&) const = default;                            int m; };

	struct SU { std::strong_ordering  operator<=>(const SU& o) const { return m <=> o.m; }              int m; };
	struct WU { std::weak_ordering    operator<=>(const WU& o) const { return m <=> o.m; }              int m; };
	struct PU { std::partial_ordering operator<=>(const PU& o) const { return PartiallyOrder(m, o.m); } int m; };

	AssertIsComparableAtTheSameTime<X>();

	AssertIsComparableAtTheSameTime<Eq>();
	AssertIsComparableAtTheSameTime<Ne>();
	AssertIsComparableAtTheSameTime<Lt>();
	AssertIsComparableAtTheSameTime<Gt>();
	AssertIsComparableAtTheSameTime<Le>();
	AssertIsComparableAtTheSameTime<Ge>();

	AssertIsComparableAtTheSameTime<S>();
	AssertIsComparableAtTheSameTime<W>();
	AssertIsComparableAtTheSameTime<P>();

	AssertIsComparableAtTheSameTime<SU>();
	AssertIsComparableAtTheSameTime<WU>();
	AssertIsComparableAtTheSameTime<PU>();
}


TEST(Ref, Hashing)
{
	struct X {};

	struct Y {
		BF::Hash BF_GetHash() const { return {}; }
	};

	struct Z {
		BF::Hash BF_GetHash() const { return { 123 }; }
	};

	AssertIsHashableAtTheSameTime<X>();
	AssertIsHashableAtTheSameTime<Y>();
	AssertIsHashableAtTheSameTime<Z>();
}
