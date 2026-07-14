#include "BF/Ref.hpp"

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
