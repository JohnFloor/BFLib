#include "BF/FunctionRef.hpp"

#include <functional>
#include <vector>
#include "gtest/gtest.h"
#include "BF/TestUtils.hpp"


namespace {


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////// Helpers /////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


struct FunctorArchetype : BF::ImmobileClass, BF::PoisonedAddrOpClass {};


template <class Signature>
struct Single;

template <class Ret, class... Pars>
struct Single<Ret (Pars...)> {
	Ret operator()(Pars...) const noexcept			{ return Ret(); }

	static Ret  SF(Pars...) noexcept				{ return Ret(); }
};


template <class>
struct DummyPar {};


template <class Signature1, class Signature2 = void (DummyPar<Signature1>)>
struct Double;

template <class Ret1, class... Pars1, class Ret2, class... Pars2>
struct Double<Ret1 (Pars1...), Ret2 (Pars2...)> {
	Ret1 operator()(Pars1...) const noexcept		{ return (Ret1) 1; }
	Ret2 operator()(Pars2...) const noexcept		{ return (Ret2) 2; }

	static Ret1  SF(Pars1...) noexcept				{ return (Ret1) 1; }
	static Ret2  SF(Pars2...) noexcept				{ return (Ret2) 2; }
};


struct Counter {
	void operator()() noexcept { count++; }

	int count = 0;
};


enum class Qual {
	X,  C,   V,   CV,
	R,  CR,  VR,  CVR,
	RR, CRR, VRR, CVRR
};


template <class Signature, class Initializer, Qual ExpectedQual>
static void TestOneCall()
{
	// If 'BF::FunctionRef<Signature>' is initialized with 'Initializer', expect to call 'ExpectedQual'.

	std::decay_t<Initializer> in1;
	BF::FunctionRef<Signature> f1 = static_cast<Initializer&&>(in1);	// 'f1' is initialized
	EXPECT_EQ(f1(), ExpectedQual);

	std::decay_t<Initializer> in2;
	BF::FunctionRef<Signature> f2;
	f2 = static_cast<Initializer&&>(in2);								// 'f2' is assigned
	EXPECT_EQ(f2(), ExpectedQual);
}


template <class DestinationSignature, class SourceSignature = DestinationSignature>
static void TestCopyMove()
{
	struct S {
		static int Fun() noexcept { return 123; }
	};

	using DestFun   = BF::FunctionRef<DestinationSignature>;
	using SourceFun = BF::MemsetValue<BF::FunctionRef<SourceSignature>>;

	{ DestFun    f = SourceFun(&S::Fun).GetL();  EXPECT_EQ(f(), 123); }
	{ DestFun f; f = SourceFun(&S::Fun).GetL();  EXPECT_EQ(f(), 123); }
	{ DestFun    f = SourceFun(&S::Fun).GetCL(); EXPECT_EQ(f(), 123); }
	{ DestFun f; f = SourceFun(&S::Fun).GetCL(); EXPECT_EQ(f(), 123); }
	{ DestFun    f = SourceFun(&S::Fun).GetR();  EXPECT_EQ(f(), 123); }
	{ DestFun f; f = SourceFun(&S::Fun).GetR();  EXPECT_EQ(f(), 123); }
	{ DestFun    f = SourceFun(&S::Fun).GetCR(); EXPECT_EQ(f(), 123); }
	{ DestFun f; f = SourceFun(&S::Fun).GetCR(); EXPECT_EQ(f(), 123); }

	static_assert(std::is_trivially_copyable_v<DestFun>);

	static_assert(std::is_trivially_constructible_v<DestFun, DestFun&>);
	static_assert(std::is_trivially_constructible_v<DestFun, const DestFun&>);
	static_assert(std::is_trivially_constructible_v<DestFun, DestFun&&>);
	static_assert(std::is_trivially_constructible_v<DestFun, const DestFun&&>);

	static_assert(std::is_trivially_assignable_v<DestFun&, DestFun&>);
	static_assert(std::is_trivially_assignable_v<DestFun&, const DestFun&>);
	static_assert(std::is_trivially_assignable_v<DestFun&, DestFun&&>);
	static_assert(std::is_trivially_assignable_v<DestFun&, const DestFun&&>);

	static_assert(std::is_trivially_destructible_v<DestFun>);
}


}	// namespace


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////// std::function demos /////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


TEST(StdFunctionDemo, StdRef)
{
	struct S {
		static void ThreeTimes(const std::function<void ()>& fun)	{ fun(); fun(); fun(); }
	};

	int i = 0;
	S::ThreeTimes([=] () mutable { i++; });						// captures by copy, 'i' is incremented in the temporary std::function
	EXPECT_EQ(i, 0);

	S::ThreeTimes([&] { i++; });								// captures by reference, the local 'i' is incremented
	EXPECT_EQ(i, 3);

	Counter c;
	S::ThreeTimes(c);											// 'count' is incremented in the temporary std::function
	EXPECT_EQ(c.count, 0);

	S::ThreeTimes(std::ref(c));									// with std::ref() 'c' isn't copied or moved
	EXPECT_EQ(c.count, 3);
}


TEST(StdFunctionDemo, Conversion)
{
	struct Fruit {};
	struct Apple : Fruit {};

	struct S {
		static Apple FruitToApple(const Fruit&) { return {}; }
	};

	std::function<Fruit (const Apple&)> fun = &S::FruitToApple;	// parameter and return types are converted silently

	Apple a;
	Fruit f = fun(a);
}


TEST(StdFunctionDemo, ConstIncorrectness)						// https://wg21.link/N4348
{
	Counter c;
	const std::function<void ()> f = c;

	f();

	EXPECT_EQ(f.target<Counter>()->count, 1);					// 'f' is const, but 'count' is modified in it
}


TEST(StdFunctionDemo, FromOverloaded)
{
	using Doub = Double<int (), int (bool)>;

	// It's not possible to infer the template argument of the templated ctor./assignment from an 'overloaded-function'.
	// The 'auto' example below works the same way, and it also doesn't compile.

//	{ std::function<int ()>    f = &Doub::SF; }			// error
//	{ std::function<int ()> f; f = &Doub::SF; }			// error
//	{ auto fail = &Doub::SF; }							// error: cannot deduce type for 'auto' from 'overloaded-function'

	// This works. The template argument can be inferred as 'Doub', and 'Doub' is callable with the given arguments.

	{ Doub d; std::function<int ()>        f = d;   EXPECT_EQ(f(),      1); }
	{ Doub d; std::function<int ()>     f; f = d;   EXPECT_EQ(f(),      1); }
	{ Doub d; std::function<int (bool)>    f = d;   EXPECT_EQ(f(false), 2); }
	{ Doub d; std::function<int (bool)> f; f = d;   EXPECT_EQ(f(false), 2); }
}


TEST(StdFunctionDemo, OverloadingOnCallableSignature_Convertible)
{
	struct S {
		int	Enumerate(const std::function<void ()>&)		{ return 1; }
		int	Enumerate(const std::function<bool ()>&)		{ return 2; }
	};

	S s;

	EXPECT_EQ(s.Enumerate(std::function<void ()>{}), 1);
	EXPECT_EQ(s.Enumerate(std::function<bool ()>{}), 2);

	EXPECT_EQ(s.Enumerate(Single<void ()>::SF), 1);
//	s.Enumerate(Single<bool ()>::SF);					// error (ambiguous)

	EXPECT_EQ(s.Enumerate(Single<void ()>{}), 1);
//	s.Enumerate(Single<bool ()>{});						// error (ambiguous)

//	s.Enumerate(Double<void ()>::SF);					// error (won't initialize from 'overloaded-function')
//	s.Enumerate(Double<bool ()>::SF);					// error (won't initialize from 'overloaded-function')

	EXPECT_EQ(s.Enumerate(Double<void ()>{}), 1);
//	s.Enumerate(Double<bool ()>{});						// error (ambiguous)
}


TEST(StdFunctionDemo, OverloadingOnCallableSignature_NotConvertible)
{
	struct S {
		int	Enumerate(const std::function<short* ()>&)		{ return 1; }
		int	Enumerate(const std::function<long* ()>&)		{ return 2; }
	};

	S s;

	EXPECT_EQ(s.Enumerate(std::function<short* ()>{}), 1);
	EXPECT_EQ(s.Enumerate(std::function<long* ()>{}),  2);

	EXPECT_EQ(s.Enumerate(Single<short* ()>::SF), 1);
	EXPECT_EQ(s.Enumerate(Single<long* ()>::SF),  2);

	EXPECT_EQ(s.Enumerate(Single<short* ()>{}), 1);
	EXPECT_EQ(s.Enumerate(Single<long* ()>{}),  2);

//	s.Enumerate(Double<short* ()>::SF);					// error (won't initialize from 'overloaded-function')
//	s.Enumerate(Double<long* ()>::SF);					// error (won't initialize from 'overloaded-function')

	EXPECT_EQ(s.Enumerate(Double<short* ()>{}), 1);
	EXPECT_EQ(s.Enumerate(Double<long* ()>{}),  2);
}


TEST(StdFunctionDemo, OverloadingOnCallableSignature_Const)
{
	struct S {
		int	Enumerate(const std::function<void (double&)>&)				{ return 1; }
		int	Enumerate(const std::function<void (const double&)>&) const	{ return 2; }
	};

	S s;

	EXPECT_EQ(s.Enumerate(std::function<void (double&)>{}), 1);
//	s.Enumerate(std::function<void (const double&)>{});				// error: overloaded functions have similar conversions

	EXPECT_EQ(s.Enumerate(Single<void (double&)>::SF),       1);
	EXPECT_EQ(s.Enumerate(Single<void (const double&)>::SF), 1);	// surprise! (calls #1)

	EXPECT_EQ(s.Enumerate(Single<void (double&)>{}),       1);
	EXPECT_EQ(s.Enumerate(Single<void (const double&)>{}), 1);		// surprise! (calls #1)

//	s.Enumerate(Double<void (double&)>::SF);						// error (won't initialize from 'overloaded-function')
//	s.Enumerate(Double<void (const double&)>::SF);					// error (won't initialize from 'overloaded-function')

	EXPECT_EQ(s.Enumerate(Double<void (double&)>{}),       1);
	EXPECT_EQ(s.Enumerate(Double<void (const double&)>{}), 1);		// surprise! (calls #1)

	const S cs;

	EXPECT_EQ(cs.Enumerate(BF::FunctionRef<void (const double&)>{}), 2);
	EXPECT_EQ(cs.Enumerate(Single<void (const double&)>::SF), 2);
	EXPECT_EQ(cs.Enumerate(Single<void (const double&)>{}),   2);
//	cs.Enumerate(Double<void (const double&)>::SF);					// error (won't initialize from 'overloaded-function')
	EXPECT_EQ(cs.Enumerate(Double<void (const double&)>{}),   2);
}


TEST(StdFunctionDemo, OverloadingOnCallableSignature_ConstConvertible)
{
	// "Const" demo is bad enough. No need to demonstrate this.
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////// Test cases //////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


TEST(FunctionRef, Inheritance)
{
//	class C1 : BF::FunctionRef<int ()> {};					// [CompilationError]: it has been declared as 'final'
//	class C2 : BF::FunctionRef<int () const> {};			// [CompilationError]: it has been declared as 'final'
//	class C3 : BF::FunctionRef<int () noexcept> {};			// [CompilationError]: it has been declared as 'final'
//	class C4 : BF::FunctionRef<int () const noexcept> {};	// [CompilationError]: it has been declared as 'final'
}


TEST(FunctionRef, FromOverloaded)
{
	using Doub = Double<int (), int (bool)>;

	{         BF::FunctionRef<int ()>        f = &Doub::SF; EXPECT_EQ(f(),      1); }
	{         BF::FunctionRef<int ()>     f; f = &Doub::SF; EXPECT_EQ(f(),      1); }
	{         BF::FunctionRef<int (bool)>    f = &Doub::SF; EXPECT_EQ(f(false), 2); }
	{         BF::FunctionRef<int (bool)> f; f = &Doub::SF; EXPECT_EQ(f(false), 2); }

	{ Doub d; BF::FunctionRef<int ()>        f = d;         EXPECT_EQ(f(),      1); }
	{ Doub d; BF::FunctionRef<int ()>     f; f = d;         EXPECT_EQ(f(),      1); }
	{ Doub d; BF::FunctionRef<int (bool)>    f = d;         EXPECT_EQ(f(false), 2); }
	{ Doub d; BF::FunctionRef<int (bool)> f; f = d;         EXPECT_EQ(f(false), 2); }
}


TEST(FunctionRef, OverloadingOnCallableSignature_Convertible)
{
	struct S {
		int	Enumerate(BF::FunctionRef<void ()>)		{ return 1; }
		int	Enumerate(BF::FunctionRef<bool ()>)		{ return 2; }
	};

	S s;

	EXPECT_EQ(s.Enumerate(BF::FunctionRef<void ()>{}), 1);
	EXPECT_EQ(s.Enumerate(BF::FunctionRef<bool ()>{}), 2);

	EXPECT_EQ(s.Enumerate(Single<void ()>::SF),        1);
	EXPECT_EQ(s.Enumerate(Single<bool ()>::SF),        2);

	EXPECT_EQ(s.Enumerate(Single<void ()>{}),          1);
	EXPECT_EQ(s.Enumerate(Single<bool ()>{}),          2);

	EXPECT_EQ(s.Enumerate(Double<void ()>::SF),        1);
	EXPECT_EQ(s.Enumerate(Double<bool ()>::SF),        2);

	EXPECT_EQ(s.Enumerate(Double<void ()>{}),          1);
	EXPECT_EQ(s.Enumerate(Double<bool ()>{}),          2);
}


TEST(FunctionRef, OverloadingOnCallableSignature_NotConvertible)
{
	// "Convertible" passes for BF::FunctionRef.
	// "Convertible" test subsumes "NotConvertible". No need to test this separately.
}


TEST(FunctionRef, OverloadingOnCallableSignature_Const)
{
	struct S {
		int	Enumerate(BF::FunctionRef<void (double&)>)				{ return 1; }
		int	Enumerate(BF::FunctionRef<void (const double&)>) const	{ return 2; }
	};

	S s;

	EXPECT_EQ(s.Enumerate(BF::FunctionRef<void (double&)>{}),        1);
	EXPECT_EQ(s.Enumerate(BF::FunctionRef<void (const double&)>{}),  2);

	EXPECT_EQ(s.Enumerate(Single<void (double&)>::SF),               1);
	EXPECT_EQ(s.Enumerate(Single<void (const double&)>::SF),         2);

	EXPECT_EQ(s.Enumerate(Single<void (double&)>{}),                 1);
	EXPECT_EQ(s.Enumerate(Single<void (const double&)>{}),           2);

	EXPECT_EQ(s.Enumerate(Double<void (double&)>::SF),               1);
	EXPECT_EQ(s.Enumerate(Double<void (const double&)>::SF),         2);

	EXPECT_EQ(s.Enumerate(Double<void (double&)>{}),                 1);
	EXPECT_EQ(s.Enumerate(Double<void (const double&)>{}),           2);

	const S cs;

	EXPECT_EQ(cs.Enumerate(BF::FunctionRef<void (const double&)>{}), 2);
	EXPECT_EQ(cs.Enumerate(Single<void (const double&)>::SF),        2);
	EXPECT_EQ(cs.Enumerate(Single<void (const double&)>{}),          2);
	EXPECT_EQ(cs.Enumerate(Double<void (const double&)>::SF),        2);
	EXPECT_EQ(cs.Enumerate(Double<void (const double&)>{}),          2);
}


TEST(FunctionRef, OverloadingOnCallableSignature_ConstConvertible)
{
	struct S {
		int	Enumerate(BF::FunctionRef<void (double&)>)				{ return 1; }
		int	Enumerate(BF::FunctionRef<void (const double&)>) const	{ return 2; }
		int	Enumerate(BF::FunctionRef<bool (double&)>)				{ return 3; }
		int	Enumerate(BF::FunctionRef<bool (const double&)>) const	{ return 4; }
	};

	S s;

	EXPECT_EQ(s.Enumerate(BF::FunctionRef<void (double&)>{}),        1);
	EXPECT_EQ(s.Enumerate(BF::FunctionRef<void (const double&)>{}),  2);
	EXPECT_EQ(s.Enumerate(BF::FunctionRef<bool (double&)>{}),        3);
	EXPECT_EQ(s.Enumerate(BF::FunctionRef<bool (const double&)>{}),  4);

	EXPECT_EQ(s.Enumerate(Single<void (double&)>::SF),               1);
	EXPECT_EQ(s.Enumerate(Single<void (const double&)>::SF),         2);
	EXPECT_EQ(s.Enumerate(Single<bool (double&)>::SF),               3);
	EXPECT_EQ(s.Enumerate(Single<bool (const double&)>::SF),         4);

	EXPECT_EQ(s.Enumerate(Single<void (double&)>{}),                 1);
	EXPECT_EQ(s.Enumerate(Single<void (const double&)>{}),           2);
	EXPECT_EQ(s.Enumerate(Single<bool (double&)>{}),                 3);
	EXPECT_EQ(s.Enumerate(Single<bool (const double&)>{}),           4);

	EXPECT_EQ(s.Enumerate(Double<void (double&)>::SF),               1);
	EXPECT_EQ(s.Enumerate(Double<void (const double&)>::SF),         2);
	EXPECT_EQ(s.Enumerate(Double<bool (double&)>::SF),               3);
	EXPECT_EQ(s.Enumerate(Double<bool (const double&)>::SF),         4);

	EXPECT_EQ(s.Enumerate(Double<void (double&)>{}),                 1);
	EXPECT_EQ(s.Enumerate(Double<void (const double&)>{}),           2);
	EXPECT_EQ(s.Enumerate(Double<bool (double&)>{}),                 3);
	EXPECT_EQ(s.Enumerate(Double<bool (const double&)>{}),           4);

	const S cs;

	EXPECT_EQ(cs.Enumerate(BF::FunctionRef<void (const double&)>{}), 2);
	EXPECT_EQ(cs.Enumerate(BF::FunctionRef<bool (const double&)>{}), 4);

	EXPECT_EQ(cs.Enumerate(Single<void (const double&)>::SF),        2);
	EXPECT_EQ(cs.Enumerate(Single<bool (const double&)>::SF),        4);

	EXPECT_EQ(cs.Enumerate(Single<void (const double&)>{}),          2);
	EXPECT_EQ(cs.Enumerate(Single<bool (const double&)>{}),          4);

	EXPECT_EQ(cs.Enumerate(Double<void (const double&)>::SF),        2);
	EXPECT_EQ(cs.Enumerate(Double<bool (const double&)>::SF),        4);

	EXPECT_EQ(cs.Enumerate(Double<void (const double&)>{}),          2);
	EXPECT_EQ(cs.Enumerate(Double<bool (const double&)>{}),          4);
}


TEST(FunctionRef, ForwardingConstNoexceptToCallOp)
{
	using X  = BF::FunctionRef<void ()>;
	using C  = BF::FunctionRef<void () const>;
	using N  = BF::FunctionRef<void () noexcept>;
	using CN = BF::FunctionRef<void () const noexcept>;

	static_assert(std::is_same_v<decltype(&X::operator()),  void (X::*)()>);
	static_assert(std::is_same_v<decltype(&C::operator()),  void (C::*)() const>);
	static_assert(std::is_same_v<decltype(&N::operator()),  void (N::*)() noexcept>);
	static_assert(std::is_same_v<decltype(&CN::operator()), void (CN::*)() const noexcept>);
}


TEST(FunctionRef, WontCallNotMatchingSignature)
{
	struct S {
		void operator()(int*) & {}
		void operator()(void*)  {}
	};

	// The below 'f' is initialized from an rvalue.
	// S::operator()(int*) cannot be called on it, because it is &-qualified.
	// S::operator()(void*) can take an int*, and can be called on an rvalue, but still won't be called, because only matching signatures are considered.

//	BF::FunctionRef<void (int*)> f = S{};					// [CompilationError]: Cannot call operator()(Pars...); the pointee would have to be an lvalue.
}


TEST(FunctionRef, CtorDefault)
{
	{ BF::FunctionRef<void ()>                f; }
	{ BF::FunctionRef<void () const>          f; }
	{ BF::FunctionRef<void () noexcept>       f; }
	{ BF::FunctionRef<void () const noexcept> f; }

//	{ BF::FunctionRef<void (...)>             f; }			// [CompilationError]: 'Signature' must be a function type in the form 'Ret (Pars...) [const] [noexcept]'.
//	{ BF::FunctionRef<void () volatile>       f; }			// [CompilationError]: 'Signature' must be a function type in the form 'Ret (Pars...) [const] [noexcept]'.
//	{ BF::FunctionRef<void () &>              f; }			// [CompilationError]: 'Signature' must be a function type in the form 'Ret (Pars...) [const] [noexcept]'.
//	{ BF::FunctionRef<void () &&>             f; }			// [CompilationError]: 'Signature' must be a function type in the form 'Ret (Pars...) [const] [noexcept]'.
}


TEST(FunctionRef, CtorBad)
{
	{ BF::FunctionRef<void ()>                f = BF::Bad; }
	{ BF::FunctionRef<void () const>          f = BF::Bad; }
	{ BF::FunctionRef<void () noexcept>       f = BF::Bad; }
	{ BF::FunctionRef<void () const noexcept> f = BF::Bad; }
}


TEST(FunctionRef, CtorUninitialized)
{
	{ BF::FunctionRef<void ()>                f = BF::Uninitialized; }
	{ BF::FunctionRef<void () const>          f = BF::Uninitialized; }
	{ BF::FunctionRef<void () noexcept>       f = BF::Uninitialized; }
	{ BF::FunctionRef<void () const noexcept> f = BF::Uninitialized; }
}


TEST(FunctionRef, CopyMove)
{
	TestCopyMove<int ()>();
	TestCopyMove<int () const>();
	TestCopyMove<int () noexcept>();
	TestCopyMove<int () const noexcept>();

	volatile BF::FunctionRef<void ()> volatileSource;
//	{ BF::FunctionRef<void ()>    f = volatileSource; }		// [CompilationError]: 'initializing': cannot convert from
//	{ BF::FunctionRef<void ()> f; f = volatileSource; }		// [CompilationError]: binary '=': no operator found which takes a right-hand operand of type
}


TEST(FunctionRef, CopyMoveFriend)
{
	TestCopyMove<int (),          int () const>();
	TestCopyMove<int (),          int () noexcept>();
	TestCopyMove<int (),          int () const noexcept>();
	TestCopyMove<int () const,    int () const noexcept>();
	TestCopyMove<int () noexcept, int () const noexcept>();

	BF::FunctionRef<void ()> source;
//	{ BF::FunctionRef<void () const>       f = source;            }	// [CompilationError]: Signature of source BF::FunctionRef should be also const.
//	{ BF::FunctionRef<void () const>    f; f = source;            }	// [CompilationError]: Signature of source BF::FunctionRef should be also const.
//	{ BF::FunctionRef<void () const>       f = std::move(source); }	// [CompilationError]: Signature of source BF::FunctionRef should be also const.
//	{ BF::FunctionRef<void () const>    f; f = std::move(source); }	// [CompilationError]: Signature of source BF::FunctionRef should be also const.
//	{ BF::FunctionRef<void () noexcept>    f = source;            }	// [CompilationError]: Signature of source BF::FunctionRef should be also noexcept.
//	{ BF::FunctionRef<void () noexcept> f; f = source;            }	// [CompilationError]: Signature of source BF::FunctionRef should be also noexcept.
//	{ BF::FunctionRef<void () noexcept>    f = std::move(source); }	// [CompilationError]: Signature of source BF::FunctionRef should be also noexcept.
//	{ BF::FunctionRef<void () noexcept> f; f = std::move(source); }	// [CompilationError]: Signature of source BF::FunctionRef should be also noexcept.

	volatile BF::FunctionRef<void () const> volatileSource;
//	{ BF::FunctionRef<void ()>    f = volatileSource;            }	// [CompilationError]: Copy-constructing/assigning from a volatile source would lose volatile qualifier.
//	{ BF::FunctionRef<void ()> f; f = volatileSource;            }	// [CompilationError]: Copy-constructing/assigning from a volatile source would lose volatile qualifier.
//	{ BF::FunctionRef<void ()>    f = std::move(volatileSource); }	// [CompilationError]: Copy-constructing/assigning from a volatile source would lose volatile qualifier.
//	{ BF::FunctionRef<void ()> f; f = std::move(volatileSource); }	// [CompilationError]: Copy-constructing/assigning from a volatile source would lose volatile qualifier.
}


TEST(FunctionRef, FromFunction)
{
	struct S {
		static int Fun() { return 123; }
	};

	{ BF::FunctionRef<int ()>                   f = &S::Fun;   EXPECT_EQ(f(), 123); }
	{ BF::FunctionRef<int ()>                f; f = &S::Fun;   EXPECT_EQ(f(), 123); }
	{ BF::FunctionRef<int () const>             f = &S::Fun;   EXPECT_EQ(f(), 123); }
	{ BF::FunctionRef<int () const>          f; f = &S::Fun;   EXPECT_EQ(f(), 123); }
//	{ BF::FunctionRef<int () noexcept>          f = &S::Fun; }		// [CompilationError]: This BF::FunctionRef can only point to 'noexcept' functions.
//	{ BF::FunctionRef<int () noexcept>       f; f = &S::Fun; }		// [CompilationError]: This BF::FunctionRef can only point to 'noexcept' functions.
//	{ BF::FunctionRef<int () const noexcept>    f = &S::Fun; }		// [CompilationError]: This BF::FunctionRef can only point to 'noexcept' functions.
//	{ BF::FunctionRef<int () const noexcept> f; f = &S::Fun; }		// [CompilationError]: This BF::FunctionRef can only point to 'noexcept' functions.

	{ BF::FunctionRef<int ()>                   f =  S::Fun;   EXPECT_EQ(f(), 123); }
	{ BF::FunctionRef<int ()>                f; f =  S::Fun;   EXPECT_EQ(f(), 123); }
	{ BF::FunctionRef<int () const>             f =  S::Fun;   EXPECT_EQ(f(), 123); }
	{ BF::FunctionRef<int () const>          f; f =  S::Fun;   EXPECT_EQ(f(), 123); }
//	{ BF::FunctionRef<int () noexcept>          f =  S::Fun; }		// [CompilationError]: This BF::FunctionRef can only point to 'noexcept' functions.
//	{ BF::FunctionRef<int () noexcept>       f; f =  S::Fun; }		// [CompilationError]: This BF::FunctionRef can only point to 'noexcept' functions.
//	{ BF::FunctionRef<int () const noexcept>    f =  S::Fun; }		// [CompilationError]: This BF::FunctionRef can only point to 'noexcept' functions.
//	{ BF::FunctionRef<int () const noexcept> f; f =  S::Fun; }		// [CompilationError]: This BF::FunctionRef can only point to 'noexcept' functions.

	struct SN {
		static int Fun() noexcept { return 123; }
	};

	{ BF::FunctionRef<int ()>                   f = &SN::Fun;  EXPECT_EQ(f(), 123); }
	{ BF::FunctionRef<int ()>                f; f = &SN::Fun;  EXPECT_EQ(f(), 123); }
	{ BF::FunctionRef<int () const>             f = &SN::Fun;  EXPECT_EQ(f(), 123); }
	{ BF::FunctionRef<int () const>          f; f = &SN::Fun;  EXPECT_EQ(f(), 123); }
	{ BF::FunctionRef<int () noexcept>          f = &SN::Fun;  EXPECT_EQ(f(), 123); }
	{ BF::FunctionRef<int () noexcept>       f; f = &SN::Fun;  EXPECT_EQ(f(), 123); }
	{ BF::FunctionRef<int () const noexcept>    f = &SN::Fun;  EXPECT_EQ(f(), 123); }
	{ BF::FunctionRef<int () const noexcept> f; f = &SN::Fun;  EXPECT_EQ(f(), 123); }

	{ BF::FunctionRef<int ()>                   f =  SN::Fun;  EXPECT_EQ(f(), 123); }
	{ BF::FunctionRef<int ()>                f; f =  SN::Fun;  EXPECT_EQ(f(), 123); }
	{ BF::FunctionRef<int () const>             f =  SN::Fun;  EXPECT_EQ(f(), 123); }
	{ BF::FunctionRef<int () const>          f; f =  SN::Fun;  EXPECT_EQ(f(), 123); }
	{ BF::FunctionRef<int () noexcept>          f =  SN::Fun;  EXPECT_EQ(f(), 123); }
	{ BF::FunctionRef<int () noexcept>       f; f =  SN::Fun;  EXPECT_EQ(f(), 123); }
	{ BF::FunctionRef<int () const noexcept>    f =  SN::Fun;  EXPECT_EQ(f(), 123); }
	{ BF::FunctionRef<int () const noexcept> f; f =  SN::Fun;  EXPECT_EQ(f(), 123); }
}


TEST(FunctionRef, FromFunctor)
{
	struct S : FunctorArchetype {
		Qual	operator()()							{ return Qual::X;  }
		Qual	operator()() const						{ return Qual::C;  }
		Qual	operator()() volatile					{ return Qual::V;  }
		Qual	operator()() const volatile				{ return Qual::CV; }
	};

	TestOneCall<Qual (),                S&,                   Qual::X> ();
	TestOneCall<Qual (),                const S&,             Qual::C> ();
	TestOneCall<Qual (),                volatile S&,          Qual::V> ();
	TestOneCall<Qual (),                const volatile S&,    Qual::CV>();
	TestOneCall<Qual (),                S&&,                  Qual::X> ();
	TestOneCall<Qual (),                const S&&,            Qual::C> ();
	TestOneCall<Qual (),                volatile S&&,         Qual::V> ();
	TestOneCall<Qual (),                const volatile S&&,   Qual::CV>();

	TestOneCall<Qual () const,          S&,                   Qual::C> ();
	TestOneCall<Qual () const,          const S&,             Qual::C> ();
	TestOneCall<Qual () const,          volatile S&,          Qual::CV>();
	TestOneCall<Qual () const,          const volatile S&,    Qual::CV>();
	TestOneCall<Qual () const,          S&&,                  Qual::C> ();
	TestOneCall<Qual () const,          const S&&,            Qual::C> ();
	TestOneCall<Qual () const,          volatile S&&,         Qual::CV>();
	TestOneCall<Qual () const,          const volatile S&&,   Qual::CV>();

	[[maybe_unused]] S s;
//	{ BF::FunctionRef<Qual () noexcept>          f = s; }		// [CompilationError]: The operator() to be called is not marked 'noexcept'.
//	{ BF::FunctionRef<Qual () noexcept>       f; f = s; }		// [CompilationError]: The operator() to be called is not marked 'noexcept'.
//	{ BF::FunctionRef<Qual () const noexcept>    f = s; }		// [CompilationError]: The operator() to be called is not marked 'noexcept'.
//	{ BF::FunctionRef<Qual () const noexcept> f; f = s; }		// [CompilationError]: The operator() to be called is not marked 'noexcept'.

	struct SN : FunctorArchetype {
		Qual	operator()() noexcept					{ return Qual::X;  }
		Qual	operator()() const noexcept				{ return Qual::C;  }
		Qual	operator()() volatile noexcept			{ return Qual::V;  }
		Qual	operator()() const volatile noexcept	{ return Qual::CV; }
	};

	TestOneCall<Qual (),                SN&,                  Qual::X> ();
	TestOneCall<Qual (),                const SN&,            Qual::C> ();
	TestOneCall<Qual (),                volatile SN&,         Qual::V> ();
	TestOneCall<Qual (),                const volatile SN&,   Qual::CV>();
	TestOneCall<Qual (),                SN&&,                 Qual::X> ();
	TestOneCall<Qual (),                const SN&&,           Qual::C> ();
	TestOneCall<Qual (),                volatile SN&&,        Qual::V> ();
	TestOneCall<Qual (),                const volatile SN&&,  Qual::CV>();

	TestOneCall<Qual () const,          SN&,                  Qual::C> ();
	TestOneCall<Qual () const,          const SN&,            Qual::C> ();
	TestOneCall<Qual () const,          volatile SN&,         Qual::CV>();
	TestOneCall<Qual () const,          const volatile SN&,   Qual::CV>();
	TestOneCall<Qual () const,          SN&&,                 Qual::C> ();
	TestOneCall<Qual () const,          const SN&&,           Qual::C> ();
	TestOneCall<Qual () const,          volatile SN&&,        Qual::CV>();
	TestOneCall<Qual () const,          const volatile SN&&,  Qual::CV>();

	TestOneCall<Qual () noexcept,       SN&,                  Qual::X> ();
	TestOneCall<Qual () noexcept,       const SN&,            Qual::C> ();
	TestOneCall<Qual () noexcept,       volatile SN&,         Qual::V> ();
	TestOneCall<Qual () noexcept,       const volatile SN&,   Qual::CV>();
	TestOneCall<Qual () noexcept,       SN&&,                 Qual::X> ();
	TestOneCall<Qual () noexcept,       const SN&&,           Qual::C> ();
	TestOneCall<Qual () noexcept,       volatile SN&&,        Qual::V> ();
	TestOneCall<Qual () noexcept,       const volatile SN&&,  Qual::CV>();

	TestOneCall<Qual () const noexcept, SN&,                  Qual::C> ();
	TestOneCall<Qual () const noexcept, const SN&,            Qual::C> ();
	TestOneCall<Qual () const noexcept, volatile SN&,         Qual::CV>();
	TestOneCall<Qual () const noexcept, const volatile SN&,   Qual::CV>();
	TestOneCall<Qual () const noexcept, SN&&,                 Qual::C> ();
	TestOneCall<Qual () const noexcept, const SN&&,           Qual::C> ();
	TestOneCall<Qual () const noexcept, volatile SN&&,        Qual::CV>();
	TestOneCall<Qual () const noexcept, const volatile SN&&,  Qual::CV>();

	struct SR : FunctorArchetype {
		Qual	operator()() &							{ return Qual::R;    }
		Qual	operator()() const&						{ return Qual::CR;   }
		Qual	operator()() volatile&					{ return Qual::VR;   }
		Qual	operator()() const volatile&			{ return Qual::CVR;  }
		Qual	operator()() &&							{ return Qual::RR;   }
		Qual	operator()() const&&					{ return Qual::CRR;  }
		Qual	operator()() volatile&&					{ return Qual::VRR;  }
		Qual	operator()() const volatile&&			{ return Qual::CVRR; }
	};

	TestOneCall<Qual (),                SR&,                  Qual::R>   ();
	TestOneCall<Qual (),                const SR&,            Qual::CR>  ();
	TestOneCall<Qual (),                volatile SR&,         Qual::VR>  ();
	TestOneCall<Qual (),                const volatile SR&,   Qual::CVR> ();
	TestOneCall<Qual (),                SR&&,                 Qual::RR>  ();
	TestOneCall<Qual (),                const SR&&,           Qual::CRR> ();
	TestOneCall<Qual (),                volatile SR&&,        Qual::VRR> ();
	TestOneCall<Qual (),                const volatile SR&&,  Qual::CVRR>();

	TestOneCall<Qual () const,          SR&,                  Qual::CR>  ();
	TestOneCall<Qual () const,          const SR&,            Qual::CR>  ();
	TestOneCall<Qual () const,          volatile SR&,         Qual::CVR> ();
	TestOneCall<Qual () const,          const volatile SR&,   Qual::CVR> ();
	TestOneCall<Qual () const,          SR&&,                 Qual::CRR> ();
	TestOneCall<Qual () const,          const SR&&,           Qual::CRR> ();
	TestOneCall<Qual () const,          volatile SR&&,        Qual::CVRR>();
	TestOneCall<Qual () const,          const volatile SR&&,  Qual::CVRR>();

	[[maybe_unused]] SR sr;
//	{ BF::FunctionRef<Qual () noexcept>          f = sr; }		// [CompilationError]: The operator() to be called is not marked 'noexcept'.
//	{ BF::FunctionRef<Qual () noexcept>       f; f = sr; }		// [CompilationError]: The operator() to be called is not marked 'noexcept'.
//	{ BF::FunctionRef<Qual () const noexcept>    f = sr; }		// [CompilationError]: The operator() to be called is not marked 'noexcept'.
//	{ BF::FunctionRef<Qual () const noexcept> f; f = sr; }		// [CompilationError]: The operator() to be called is not marked 'noexcept'.

	struct SRN : FunctorArchetype {
		Qual	operator()() & noexcept					{ return Qual::R;    }
		Qual	operator()() const& noexcept			{ return Qual::CR;   }
		Qual	operator()() volatile& noexcept			{ return Qual::VR;   }
		Qual	operator()() const volatile& noexcept	{ return Qual::CVR;  }
		Qual	operator()() && noexcept				{ return Qual::RR;   }
		Qual	operator()() const&& noexcept			{ return Qual::CRR;  }
		Qual	operator()() volatile&& noexcept		{ return Qual::VRR;  }
		Qual	operator()() const volatile&& noexcept	{ return Qual::CVRR; }
	};

	TestOneCall<Qual (),                SRN&,                 Qual::R>   ();
	TestOneCall<Qual (),                const SRN&,           Qual::CR>  ();
	TestOneCall<Qual (),                volatile SRN&,        Qual::VR>  ();
	TestOneCall<Qual (),                const volatile SRN&,  Qual::CVR> ();
	TestOneCall<Qual (),                SRN&&,                Qual::RR>  ();
	TestOneCall<Qual (),                const SRN&&,          Qual::CRR> ();
	TestOneCall<Qual (),                volatile SRN&&,       Qual::VRR> ();
	TestOneCall<Qual (),                const volatile SRN&&, Qual::CVRR>();

	TestOneCall<Qual () const,          SRN&,                 Qual::CR>  ();
	TestOneCall<Qual () const,          const SRN&,           Qual::CR>  ();
	TestOneCall<Qual () const,          volatile SRN&,        Qual::CVR> ();
	TestOneCall<Qual () const,          const volatile SRN&,  Qual::CVR> ();
	TestOneCall<Qual () const,          SRN&&,                Qual::CRR> ();
	TestOneCall<Qual () const,          const SRN&&,          Qual::CRR> ();
	TestOneCall<Qual () const,          volatile SRN&&,       Qual::CVRR>();
	TestOneCall<Qual () const,          const volatile SRN&&, Qual::CVRR>();

	TestOneCall<Qual () noexcept,       SRN&,                 Qual::R>   ();
	TestOneCall<Qual () noexcept,       const SRN&,           Qual::CR>  ();
	TestOneCall<Qual () noexcept,       volatile SRN&,        Qual::VR>  ();
	TestOneCall<Qual () noexcept,       const volatile SRN&,  Qual::CVR> ();
	TestOneCall<Qual () noexcept,       SRN&&,                Qual::RR>  ();
	TestOneCall<Qual () noexcept,       const SRN&&,          Qual::CRR> ();
	TestOneCall<Qual () noexcept,       volatile SRN&&,       Qual::VRR> ();
	TestOneCall<Qual () noexcept,       const volatile SRN&&, Qual::CVRR>();

	TestOneCall<Qual () const noexcept, SRN&,                 Qual::CR>  ();
	TestOneCall<Qual () const noexcept, const SRN&,           Qual::CR>  ();
	TestOneCall<Qual () const noexcept, volatile SRN&,        Qual::CVR> ();
	TestOneCall<Qual () const noexcept, const volatile SRN&,  Qual::CVR> ();
	TestOneCall<Qual () const noexcept, SRN&&,                Qual::CRR> ();
	TestOneCall<Qual () const noexcept, const SRN&&,          Qual::CRR> ();
	TestOneCall<Qual () const noexcept, volatile SRN&&,       Qual::CVRR>();
	TestOneCall<Qual () const noexcept, const volatile SRN&&, Qual::CVRR>();
}


TEST(FunctionRef, FromFunctorFailed)
{
	struct NotC {
		void operator()() {}
		void operator()() volatile {}
	};

	struct NotV {
		void operator()() {}
		void operator()() const {}
	};

	struct NotCV {
		void operator()() {}
		void operator()() const {}
		void operator()() volatile {}
	};

	[[maybe_unused]] NotC                   notC1;
	[[maybe_unused]] const NotC             notC2;
//	{ BF::FunctionRef<void () const>    f = notC1;  }		// [CompilationError]: Cannot call operator()(Pars...); the pointee would lose some cv-qualifiers.
//	{ BF::FunctionRef<void () const> f; f = notC1;  }		// [CompilationError]: Cannot call operator()(Pars...); the pointee would lose some cv-qualifiers.
//	{ BF::FunctionRef<void ()>          f = notC2;  }		// [CompilationError]: Cannot call operator()(Pars...); the pointee would lose some cv-qualifiers.
//	{ BF::FunctionRef<void ()>       f; f = notC2;  }		// [CompilationError]: Cannot call operator()(Pars...); the pointee would lose some cv-qualifiers.

	[[maybe_unused]] volatile NotV          notV;
//	{ BF::FunctionRef<void ()>          f = notV;   }		// [CompilationError]: Cannot call operator()(Pars...); the pointee would lose some cv-qualifiers.
//	{ BF::FunctionRef<void ()>       f; f = notV;   }		// [CompilationError]: Cannot call operator()(Pars...); the pointee would lose some cv-qualifiers.

	[[maybe_unused]] volatile NotCV         notCV1;
	[[maybe_unused]] const volatile NotCV   notCV2;
//	{ BF::FunctionRef<void () const>    f = notCV1; }		// [CompilationError]: Cannot call operator()(Pars...); the pointee would lose some cv-qualifiers.
//	{ BF::FunctionRef<void () const> f; f = notCV1; }		// [CompilationError]: Cannot call operator()(Pars...); the pointee would lose some cv-qualifiers.
//	{ BF::FunctionRef<void ()>          f = notCV2; }		// [CompilationError]: Cannot call operator()(Pars...); the pointee would lose some cv-qualifiers.
//	{ BF::FunctionRef<void ()>       f; f = notCV2; }		// [CompilationError]: Cannot call operator()(Pars...); the pointee would lose some cv-qualifiers.

	struct OnlyL {
		void operator()() & {}
	};

//	{ BF::FunctionRef<void ()>          f = OnlyL{}; }		// [CompilationError]: Cannot call operator()(Pars...); the pointee would have to be an lvalue.
//	{ BF::FunctionRef<void ()>       f; f = OnlyL{}; }		// [CompilationError]: Cannot call operator()(Pars...); the pointee would have to be an lvalue.

	struct OnlyR {
		void operator()() && {}
	};

	[[maybe_unused]] OnlyR                  onlyR;
//	{ BF::FunctionRef<void ()>          f = onlyR; }		// [CompilationError]: Cannot call operator()(Pars...); the pointee would have to be an rvalue.
//	{ BF::FunctionRef<void ()>       f; f = onlyR; }		// [CompilationError]: Cannot call operator()(Pars...); the pointee would have to be an rvalue.

	struct NotCL {
		void operator()() & {}
		void operator()() volatile& {}
	};

	struct NotVL {
		void operator()() & {}
		// no const& overload, as it can take rvalues
	};

	struct NotCVL {
		void operator()() & {}
		void operator()() volatile& {}
		// no const& overload, as it can take rvalues
	};

	[[maybe_unused]] NotCL                  notCL1;
	[[maybe_unused]] const NotCL            notCL2;
//	{ BF::FunctionRef<void () const>    f = std::move(notCL1);  }	// [CompilationError]: Cannot call operator()(Pars...); the pointee would lose some cv-qualifiers, and would have to be an lvalue.
//	{ BF::FunctionRef<void () const> f; f = std::move(notCL1);  }	// [CompilationError]: Cannot call operator()(Pars...); the pointee would lose some cv-qualifiers, and would have to be an lvalue.
//	{ BF::FunctionRef<void ()>          f = std::move(notCL2);  }	// [CompilationError]: Cannot call operator()(Pars...); the pointee would lose some cv-qualifiers, and would have to be an lvalue.
//	{ BF::FunctionRef<void ()>       f; f = std::move(notCL2);  }	// [CompilationError]: Cannot call operator()(Pars...); the pointee would lose some cv-qualifiers, and would have to be an lvalue.

	[[maybe_unused]] volatile NotVL         notVL;
//	{ BF::FunctionRef<void ()>          f = std::move(notVL);   }	// [CompilationError]: Cannot call operator()(Pars...); the pointee would lose some cv-qualifiers, and would have to be an lvalue.
//	{ BF::FunctionRef<void ()>       f; f = std::move(notVL);   }	// [CompilationError]: Cannot call operator()(Pars...); the pointee would lose some cv-qualifiers, and would have to be an lvalue.

	[[maybe_unused]] volatile NotCVL        notCVL1;
	[[maybe_unused]] const volatile NotCVL  notCVL2;
//	{ BF::FunctionRef<void () const>    f = std::move(notCVL1); }	// [CompilationError]: Cannot call operator()(Pars...); the pointee would lose some cv-qualifiers, and would have to be an lvalue.
//	{ BF::FunctionRef<void () const> f; f = std::move(notCVL1); }	// [CompilationError]: Cannot call operator()(Pars...); the pointee would lose some cv-qualifiers, and would have to be an lvalue.
//	{ BF::FunctionRef<void ()>          f = std::move(notCVL2); }	// [CompilationError]: Cannot call operator()(Pars...); the pointee would lose some cv-qualifiers, and would have to be an lvalue.
//	{ BF::FunctionRef<void ()>       f; f = std::move(notCVL2); }	// [CompilationError]: Cannot call operator()(Pars...); the pointee would lose some cv-qualifiers, and would have to be an lvalue.

	struct NotCR {
		void operator()() && {}
		void operator()() volatile&& {}
	};

	struct NotVR {
		void operator()() && {}
		void operator()() const&& {}
	};

	struct NotCVR {
		void operator()() && {}
		void operator()() const&& {}
		void operator()() volatile&& {}
	};

	[[maybe_unused]] NotCR                  notCR1;
	[[maybe_unused]] const NotCR            notCR2;
//	{ BF::FunctionRef<void () const>    f = notCR1;  }		// [CompilationError]: Cannot call operator()(Pars...); the pointee would lose some cv-qualifiers, and would have to be an rvalue.
//	{ BF::FunctionRef<void () const> f; f = notCR1;  }		// [CompilationError]: Cannot call operator()(Pars...); the pointee would lose some cv-qualifiers, and would have to be an rvalue.
//	{ BF::FunctionRef<void ()>          f = notCR2;  }		// [CompilationError]: Cannot call operator()(Pars...); the pointee would lose some cv-qualifiers, and would have to be an rvalue.
//	{ BF::FunctionRef<void ()>       f; f = notCR2;  }		// [CompilationError]: Cannot call operator()(Pars...); the pointee would lose some cv-qualifiers, and would have to be an rvalue.

	[[maybe_unused]] volatile NotVR         notVR;
//	{ BF::FunctionRef<void ()>          f = notVR;   }		// [CompilationError]: Cannot call operator()(Pars...); the pointee would lose some cv-qualifiers, and would have to be an rvalue.
//	{ BF::FunctionRef<void ()>       f; f = notVR;   }		// [CompilationError]: Cannot call operator()(Pars...); the pointee would lose some cv-qualifiers, and would have to be an rvalue.

	[[maybe_unused]] volatile NotCVR        notCVR1;
	[[maybe_unused]] const volatile NotCVR  notCVR2;
//	{ BF::FunctionRef<void () const>    f = notCVR1; }		// [CompilationError]: Cannot call operator()(Pars...); the pointee would lose some cv-qualifiers, and would have to be an rvalue.
//	{ BF::FunctionRef<void () const> f; f = notCVR1; }		// [CompilationError]: Cannot call operator()(Pars...); the pointee would lose some cv-qualifiers, and would have to be an rvalue.
//	{ BF::FunctionRef<void ()>          f = notCVR2; }		// [CompilationError]: Cannot call operator()(Pars...); the pointee would lose some cv-qualifiers, and would have to be an rvalue.
//	{ BF::FunctionRef<void ()>       f; f = notCVR2; }		// [CompilationError]: Cannot call operator()(Pars...); the pointee would lose some cv-qualifiers, and would have to be an rvalue.

	struct Conv {
		void operator()() {}

		using Fun = void ();
		operator Fun*() const { return nullptr; }			// this won't be used, and we get a compilation error
	};

	[[maybe_unused]] const Conv       conv;
//	{ BF::FunctionRef<void ()>    f = conv; }				// [CompilationError]: Cannot call operator()(Pars...); the pointee would lose some cv-qualifiers.
//	{ BF::FunctionRef<void ()> f; f = conv; }				// [CompilationError]: Cannot call operator()(Pars...); the pointee would lose some cv-qualifiers.
}


TEST(FunctionRef, FromWrongSignature)
{
	struct S {
		static void FunInt(int)   {}
		static void FunLong(long) {}
	};

	{ std::function<void (int)>  verifyConversion = &S::FunLong; }
	{ std::function<void (long)> verifyConversion = &S::FunInt;  }

//	{ BF::FunctionRef<void (int)>     f = &S::FunLong; }	// [CompilationError]: 'initializing': cannot convert from
//	{ BF::FunctionRef<void (int)>  f; f = &S::FunLong; }	// [CompilationError]: binary '=': no operator found which takes a right-hand operand of type
//	{ BF::FunctionRef<void (long)>    f = &S::FunInt;  }	// [CompilationError]: 'initializing': cannot convert from
//	{ BF::FunctionRef<void (long)> f; f = &S::FunInt;  }	// [CompilationError]: binary '=': no operator found which takes a right-hand operand of type

	struct SInt  { void operator()(int)  {} };
	struct SLong { void operator()(long) {} };

	{ std::function<void (int)>  verifyConversion = SLong{}; }
	{ std::function<void (long)> verifyConversion = SInt{};  }

//	{ BF::FunctionRef<void (int)>     f = SLong{}; }		// [CompilationError]: 'initializing': cannot convert from
//	{ BF::FunctionRef<void (int)>  f; f = SLong{}; }		// [CompilationError]: binary '=': no operator found which takes a right-hand operand of type
//	{ BF::FunctionRef<void (long)>    f = SInt{};  }		// [CompilationError]: 'initializing': cannot convert from
//	{ BF::FunctionRef<void (long)> f; f = SInt{};  }		// [CompilationError]: binary '=': no operator found which takes a right-hand operand of type
}


TEST(FunctionRef, FromNotCallableTypes)
{
	struct S {
		void Method() {}
		int  mVar;
	};

//	{ BF::FunctionRef<void ()>      f = ""; }				// [CompilationError]: 'initializing': cannot convert from
//	{ BF::FunctionRef<void ()>   f; f = ""; }				// [CompilationError]: binary '=': no operator found which takes a right-hand operand of type
//	{ BF::FunctionRef<void ()>      f = nullptr; }			// [CompilationError]: attempting to reference a deleted function
//	{ BF::FunctionRef<void ()>   f; f = nullptr; }			// [CompilationError]: attempting to reference a deleted function
//	{ BF::FunctionRef<void ()>      f = &S::Method; }		// [CompilationError]: attempting to reference a deleted function
//	{ BF::FunctionRef<void ()>   f; f = &S::Method; }		// [CompilationError]: attempting to reference a deleted function
//	{ BF::FunctionRef<void (S*)>    f = &S::Method; }		// [CompilationError]: attempting to reference a deleted function
//	{ BF::FunctionRef<void (S*)> f; f = &S::Method; }		// [CompilationError]: attempting to reference a deleted function
//	{ BF::FunctionRef<void (S&)>    f = &S::Method; }		// [CompilationError]: attempting to reference a deleted function
//	{ BF::FunctionRef<void (S&)> f; f = &S::Method; }		// [CompilationError]: attempting to reference a deleted function
//	{ BF::FunctionRef<void ()>      f = &S::mVar; }			// [CompilationError]: attempting to reference a deleted function
//	{ BF::FunctionRef<void ()>   f; f = &S::mVar; }			// [CompilationError]: attempting to reference a deleted function
//	{ BF::FunctionRef<void (S*)>    f = &S::mVar; }			// [CompilationError]: attempting to reference a deleted function
//	{ BF::FunctionRef<void (S*)> f; f = &S::mVar; }			// [CompilationError]: attempting to reference a deleted function
//	{ BF::FunctionRef<void (S&)>    f = &S::mVar; }			// [CompilationError]: attempting to reference a deleted function
//	{ BF::FunctionRef<void (S&)> f; f = &S::mVar; }			// [CompilationError]: attempting to reference a deleted function
//	{ BF::FunctionRef<void ()>      f = (void*)nullptr; }	// [CompilationError]: 'initializing': cannot convert from
//	{ BF::FunctionRef<void ()>   f; f = (void*)nullptr; }	// [CompilationError]: binary '=': no operator found which takes a right-hand operand of type
//	{ BF::FunctionRef<void ()>      f = (int*)nullptr; }	// [CompilationError]: 'initializing': cannot convert from
//	{ BF::FunctionRef<void ()>   f; f = (int*)nullptr; }	// [CompilationError]: binary '=': no operator found which takes a right-hand operand of type
}

TEST(FunctionRef, ParameterPassing)
{
	struct MoveOnly : BF::MoveOnlyClass {
		MoveOnly(int value) : value(value) {}
		int value;
	};

	struct Immobile : BF::ImmobileClass {
		Immobile(int value) : value(value) {}
		int value;
	};

	struct S {
		static int Fun1(MoveOnly x)		{ return x.value; }
		static int Fun2(Immobile& x)	{ return x.value; }
		static int Fun3(Immobile&& x)	{ return x.value; }
	};

	struct S1 { int operator()(MoveOnly x)   { return x.value; } };
	struct S2 { int operator()(Immobile& x)  { return x.value; } };
	struct S3 { int operator()(Immobile&& x) { return x.value; } };

	{ MoveOnly x = 123;       BF::FunctionRef<int (MoveOnly)>   f = &S::Fun1;  EXPECT_EQ(f(std::move(x)), 123); }
	{ Immobile x = 123;       BF::FunctionRef<int (Immobile&)>  f = &S::Fun2;  EXPECT_EQ(f(x),            123); }
	{ Immobile x = 123;       BF::FunctionRef<int (Immobile&&)> f = &S::Fun3;  EXPECT_EQ(f(std::move(x)), 123); }

	{ MoveOnly x = 123; S1 s; BF::FunctionRef<int (MoveOnly)>   f = s;         EXPECT_EQ(f(std::move(x)), 123); }
	{ Immobile x = 123; S2 s; BF::FunctionRef<int (Immobile&)>  f = s;         EXPECT_EQ(f(x),            123); }
	{ Immobile x = 123; S3 s; BF::FunctionRef<int (Immobile&&)> f = s;         EXPECT_EQ(f(std::move(x)), 123); }

	struct MoveCtorThrows {
		MoveCtorThrows() = default;
		[[gsl::suppress("f.6")]] MoveCtorThrows(MoveCtorThrows&&) {}	// move ctor. intentionally throws
	};

	struct DtorThrows {
		DtorThrows() = default;
		~DtorThrows() noexcept(false) {}
	};

	struct SN {
		static void Fun1(MoveCtorThrows) noexcept {}
		static void Fun2(DtorThrows) noexcept {}
	};

	struct SN1 { void operator()(MoveCtorThrows) noexcept {} };
	struct SN2 { void operator()(DtorThrows) noexcept {} };

//	{        BF::FunctionRef<void (MoveCtorThrows) noexcept>    f = &SN::Fun1; }		// [CompilationError]: A parameter's move ctor. or dtor. is not 'noexcept'.
//	{        BF::FunctionRef<void (MoveCtorThrows) noexcept> f; f = &SN::Fun1; }		// [CompilationError]: A parameter's move ctor. or dtor. is not 'noexcept'.
//	{        BF::FunctionRef<void (DtorThrows) noexcept>        f = &SN::Fun2; }		// [CompilationError]: A parameter's move ctor. or dtor. is not 'noexcept'.
//	{        BF::FunctionRef<void (DtorThrows) noexcept>     f; f = &SN::Fun2; }		// [CompilationError]: A parameter's move ctor. or dtor. is not 'noexcept'.
//	{ SN1 s; BF::FunctionRef<void (MoveCtorThrows) noexcept>    f = s;         }		// [CompilationError]: A parameter's move ctor. or dtor. is not 'noexcept'.
//	{ SN1 s; BF::FunctionRef<void (MoveCtorThrows) noexcept> f; f = s;         }		// [CompilationError]: A parameter's move ctor. or dtor. is not 'noexcept'.
//	{ SN2 s; BF::FunctionRef<void (DtorThrows) noexcept>        f = s;         }		// [CompilationError]: A parameter's move ctor. or dtor. is not 'noexcept'.
//	{ SN2 s; BF::FunctionRef<void (DtorThrows) noexcept>     f; f = s;         }		// [CompilationError]: A parameter's move ctor. or dtor. is not 'noexcept'.
}


TEST(FunctionRef, ConstCast)
{
	{
		// Checking all combinations of the declaration.
		// - Some of these can't be called in evaluated context, because of static_assert's.
		// - volatile BF::FunctionRef is not usable. You can't call operator() on it.
		               BF::FunctionRef<void ()> f;
		const          BF::FunctionRef<void ()> cf;
		volatile       BF::FunctionRef<void ()> vf;
		const volatile BF::FunctionRef<void ()> cvf;

		static_assert(requires { { f.ConstCast()              } -> std::same_as<         BF::FunctionRef<void ()>&>;  });
		static_assert(requires { { cf.ConstCast()             } -> std::same_as<         BF::FunctionRef<void ()>&>;  });
		static_assert(requires { { vf.ConstCast()             } -> std::same_as<volatile BF::FunctionRef<void ()>&>;  });
		static_assert(requires { { cvf.ConstCast()            } -> std::same_as<volatile BF::FunctionRef<void ()>&>;  });

		static_assert(requires { { std::move(f).ConstCast()   } -> std::same_as<         BF::FunctionRef<void ()>&&>; });
		static_assert(requires { { std::move(cf).ConstCast()  } -> std::same_as<         BF::FunctionRef<void ()>&&>; });
		static_assert(requires { { std::move(vf).ConstCast()  } -> std::same_as<volatile BF::FunctionRef<void ()>&&>; });
		static_assert(requires { { std::move(cvf).ConstCast() } -> std::same_as<volatile BF::FunctionRef<void ()>&&>; });
	}

	{
		BF::FunctionRef<void ()>                f;
		BF::FunctionRef<void () const>          fc;
		BF::FunctionRef<void () noexcept>       fn;
		BF::FunctionRef<void () const noexcept> fcn;

		// f.ConstCast();										// [CompilationError]: No need to cast away constness, '*this' is already not const.
		// fc.ConstCast();										// [CompilationError]: No need to cast away constness, '*this' is already not const.
		// fn.ConstCast();										// [CompilationError]: No need to cast away constness, '*this' is already not const.
		// fcn.ConstCast();										// [CompilationError]: No need to cast away constness, '*this' is already not const.
	}

	{
		Counter c;

		const BF::FunctionRef<void ()>                f  = c;
		const BF::FunctionRef<void () const>          fc;
		const BF::FunctionRef<void () noexcept>       fn = c;
		const BF::FunctionRef<void () const noexcept> fcn;

		EXPECT_EQ(&f.ConstCast(), &f);
		f.ConstCast()();
		EXPECT_EQ(c.count, 1);

		// fc.ConstCast();										// [CompilationError]: No need to cast away constness, 'operator()' is const.

		EXPECT_EQ(&fn.ConstCast(), &fn);
		fn.ConstCast()();
		EXPECT_EQ(c.count, 2);

		// fcn.ConstCast();										// [CompilationError]: No need to cast away constness, 'operator()' is const.
	}

	// volatile BF::FunctionRef is not usable. You can't call operator() on it.
}


TEST(FunctionRef, ConstCastToSignature)
{
	{
		// Checking all combinations of the declaration.
		// - volatile BF::FunctionRef is not usable. You can't call operator() on it.
		               BF::FunctionRef<const int& ()> f;
		const          BF::FunctionRef<const int& ()> cf;
		volatile       BF::FunctionRef<const int& ()> vf;
		const volatile BF::FunctionRef<const int& ()> cvf;

		std::same_as<               BF::FunctionRef<int& ()>&>  decltype(auto) f1   = f.ConstCast<int& ()>();
		std::same_as<const          BF::FunctionRef<int& ()>&>  decltype(auto) cf1  = cf.ConstCast<int& ()>();
		std::same_as<volatile       BF::FunctionRef<int& ()>&>  decltype(auto) vf1  = vf.ConstCast<int& ()>();
		std::same_as<const volatile BF::FunctionRef<int& ()>&>  decltype(auto) cvf1 = cvf.ConstCast<int& ()>();

		std::same_as<               BF::FunctionRef<int& ()>&&> decltype(auto) f2   = BF::Move(f).ConstCast<int& ()>();
		std::same_as<const          BF::FunctionRef<int& ()>&&> decltype(auto) cf2  = BF::Move(cf).ConstCast<int& ()>();
		std::same_as<volatile       BF::FunctionRef<int& ()>&&> decltype(auto) vf2  = BF::Move(vf).ConstCast<int& ()>();
		std::same_as<const volatile BF::FunctionRef<int& ()>&&> decltype(auto) cvf2 = BF::Move(cvf).ConstCast<int& ()>();
	}

	{
		const auto fPtr = +[] (int& x) noexcept { x += 111; };

		BF::FunctionRef<void (int&)>                f   = fPtr;
		BF::FunctionRef<void (int&) const>          fc  = fPtr;
		BF::FunctionRef<void (int&) noexcept>       fn  = fPtr;
		BF::FunctionRef<void (int&) const noexcept> fcn = fPtr;

		EXPECT_EQ((void*) &f.ConstCast<void (const int&)>(),                  &f);
		EXPECT_EQ((void*) &fc.ConstCast<void (const int&) const>(),           &fc);
		EXPECT_EQ((void*) &fn.ConstCast<void (const int&) noexcept>(),        &fn);
		EXPECT_EQ((void*) &fcn.ConstCast<void (const int&) const noexcept>(), &fcn);

		int        i  = 0;
		const int& ci = i;

		f.ConstCast<void (const int&)>()(ci);								EXPECT_EQ(i, 111);
		fc.ConstCast<void (const int&) const>()(ci);						EXPECT_EQ(i, 222);
		fn.ConstCast<void (const int&) noexcept>()(ci);						EXPECT_EQ(i, 333);
		fcn.ConstCast<void (const int&) const noexcept>()(ci);				EXPECT_EQ(i, 444);

		std::move(f).ConstCast<void (const int&)>()(ci);					EXPECT_EQ(i, 555);
		std::move(fc).ConstCast<void (const int&) const>()(ci);				EXPECT_EQ(i, 666);
		std::move(fn).ConstCast<void (const int&) noexcept>()(ci);			EXPECT_EQ(i, 777);
		std::move(fcn).ConstCast<void (const int&) const noexcept>()(ci);	EXPECT_EQ(i, 888);
	}

	{
		struct MyVector : std::vector<double> {
			using std::vector<double>::vector;

			void Enumerate(BF::FunctionRef<void (double&)> f)              { Enumerate(f.ConstCast<void (const double&)>()); }
			void Enumerate(BF::FunctionRef<void (const double&)> f) const  { for (const double& item : *this) f(item); }
		};

		MyVector vec { 0.0, 0.0, 0.0 };

		vec.Enumerate([] (double& item) { item = 1.0; });

		EXPECT_EQ(vec[0], 1.0);
		EXPECT_EQ(vec[1], 1.0);
		EXPECT_EQ(vec[2], 1.0);
	}

//	BF::FunctionRef<void ()>{}.ConstCast<void () &>();			// [CompilationError]: 'Signature' must be a function type in the form 'Ret (Pars...) [const] [noexcept]'.

//	BF::FunctionRef<void () const>{}.ConstCast<void ()>();		// [CompilationError]: 'Signature' of BF::FunctionRef is const, therefore 'ToSignature' of ConstCast() should also be const.
//	BF::FunctionRef<void ()>{}.ConstCast<void () const>();		// [CompilationError]: 'Signature' of BF::FunctionRef is not const, therefore 'ToSignature' of ConstCast() should also be not const.

//	BF::FunctionRef<void () noexcept>{}.ConstCast<void ()>();	// [CompilationError]: 'Signature' of BF::FunctionRef is noexcept, therefore 'ToSignature' of ConstCast() should also be noexcept.
//	BF::FunctionRef<void ()>{}.ConstCast<void () noexcept>();	// [CompilationError]: 'Signature' of BF::FunctionRef is not noexcept, therefore 'ToSignature' of ConstCast() should also be not noexcept.

//	BF::FunctionRef<int (int)>{}.ConstCast<int ()>();			// [CompilationError]: 'ToSignature' of ConstCast() should contain the same number of parameters as 'Signature' of BF::FunctionRef.
//	BF::FunctionRef<int (int)>{}.ConstCast<bool (int)>();		// [CompilationError]: ConstCast() can only change the constness of the return type.
//	BF::FunctionRef<int (int)>{}.ConstCast<int (bool)>();		// [CompilationError]: ConstCast() can only change the constness of the parameter types.
}


TEST(FunctionRef, DeductionGuide)
{
	struct S {
		static int Fun()           { return 123; }
		static int FunN() noexcept { return 123; }

		static void Overloaded();
		static void Overloaded(int);

		void Method() {}
		int  mVar;
	};

	{ BF::FunctionRef f = &S::Fun;  EXPECT_EQ(f(), 123); static_assert(std::is_same_v<decltype(f), BF::FunctionRef<int ()         >>); }
	{ BF::FunctionRef f =  S::Fun;  EXPECT_EQ(f(), 123); static_assert(std::is_same_v<decltype(f), BF::FunctionRef<int ()         >>); }

	{ BF::FunctionRef f = &S::FunN; EXPECT_EQ(f(), 123); static_assert(std::is_same_v<decltype(f), BF::FunctionRef<int () noexcept>>); }
	{ BF::FunctionRef f =  S::FunN; EXPECT_EQ(f(), 123); static_assert(std::is_same_v<decltype(f), BF::FunctionRef<int () noexcept>>); }

//	{ BF::FunctionRef f = &S::Overloaded; }		// [CompilationError]: cannot deduce template arguments
//	{ BF::FunctionRef f =  S::Overloaded; }		// [CompilationError]: cannot deduce template arguments

	struct X     : FunctorArchetype { int operator()()                           { return 123; } };
	struct C     : FunctorArchetype { int operator()() const                     { return 123; } };
	struct V     : FunctorArchetype { int operator()() volatile                  { return 123; } };
	struct CV    : FunctorArchetype { int operator()() const volatile            { return 123; } };
	struct R     : FunctorArchetype { int operator()() &                         { return 123; } };
	struct CR    : FunctorArchetype { int operator()() const&                    { return 123; } };
	struct VR    : FunctorArchetype { int operator()() volatile&                 { return 123; } };
	struct CVR   : FunctorArchetype { int operator()() const volatile&           { return 123; } };
	struct RR    : FunctorArchetype { int operator()() &&                        { return 123; } };
	struct CRR   : FunctorArchetype { int operator()() const&&                   { return 123; } };
	struct VRR   : FunctorArchetype { int operator()() volatile&&                { return 123; } };
	struct CVRR  : FunctorArchetype { int operator()() const volatile&&          { return 123; } };
	struct N     : FunctorArchetype { int operator()()                  noexcept { return 123; } };
	struct CN    : FunctorArchetype { int operator()() const            noexcept { return 123; } };
	struct VN    : FunctorArchetype { int operator()() volatile         noexcept { return 123; } };
	struct CVN   : FunctorArchetype { int operator()() const volatile   noexcept { return 123; } };
	struct RN    : FunctorArchetype { int operator()() &                noexcept { return 123; } };
	struct CRN   : FunctorArchetype { int operator()() const&           noexcept { return 123; } };
	struct VRN   : FunctorArchetype { int operator()() volatile&        noexcept { return 123; } };
	struct CVRN  : FunctorArchetype { int operator()() const volatile&  noexcept { return 123; } };
	struct RRN   : FunctorArchetype { int operator()() &&               noexcept { return 123; } };
	struct CRRN  : FunctorArchetype { int operator()() const&&          noexcept { return 123; } };
	struct VRRN  : FunctorArchetype { int operator()() volatile&&       noexcept { return 123; } };
	struct CVRRN : FunctorArchetype { int operator()() const volatile&& noexcept { return 123; } };

	{ X     s; BF::FunctionRef f = s;            EXPECT_EQ(f(), 123); static_assert(std::is_same_v<decltype(f), BF::FunctionRef<int ()               >>); }
	{ C     s; BF::FunctionRef f = s;            EXPECT_EQ(f(), 123); static_assert(std::is_same_v<decltype(f), BF::FunctionRef<int () const         >>); }
	{ V     s; BF::FunctionRef f = s;            EXPECT_EQ(f(), 123); static_assert(std::is_same_v<decltype(f), BF::FunctionRef<int ()               >>); }
	{ CV    s; BF::FunctionRef f = s;            EXPECT_EQ(f(), 123); static_assert(std::is_same_v<decltype(f), BF::FunctionRef<int () const         >>); }
	{ R     s; BF::FunctionRef f = s;            EXPECT_EQ(f(), 123); static_assert(std::is_same_v<decltype(f), BF::FunctionRef<int ()               >>); }
	{ CR    s; BF::FunctionRef f = s;            EXPECT_EQ(f(), 123); static_assert(std::is_same_v<decltype(f), BF::FunctionRef<int () const         >>); }
	{ VR    s; BF::FunctionRef f = s;            EXPECT_EQ(f(), 123); static_assert(std::is_same_v<decltype(f), BF::FunctionRef<int ()               >>); }
	{ CVR   s; BF::FunctionRef f = s;            EXPECT_EQ(f(), 123); static_assert(std::is_same_v<decltype(f), BF::FunctionRef<int () const         >>); }
	{ RR    s; BF::FunctionRef f = std::move(s); EXPECT_EQ(f(), 123); static_assert(std::is_same_v<decltype(f), BF::FunctionRef<int ()               >>); }
	{ CRR   s; BF::FunctionRef f = std::move(s); EXPECT_EQ(f(), 123); static_assert(std::is_same_v<decltype(f), BF::FunctionRef<int () const         >>); }
	{ VRR   s; BF::FunctionRef f = std::move(s); EXPECT_EQ(f(), 123); static_assert(std::is_same_v<decltype(f), BF::FunctionRef<int ()               >>); }
	{ CVRR  s; BF::FunctionRef f = std::move(s); EXPECT_EQ(f(), 123); static_assert(std::is_same_v<decltype(f), BF::FunctionRef<int () const         >>); }
	{ N     s; BF::FunctionRef f = s;            EXPECT_EQ(f(), 123); static_assert(std::is_same_v<decltype(f), BF::FunctionRef<int ()       noexcept>>); }
	{ CN    s; BF::FunctionRef f = s;            EXPECT_EQ(f(), 123); static_assert(std::is_same_v<decltype(f), BF::FunctionRef<int () const noexcept>>); }
	{ VN    s; BF::FunctionRef f = s;            EXPECT_EQ(f(), 123); static_assert(std::is_same_v<decltype(f), BF::FunctionRef<int ()       noexcept>>); }
	{ CVN   s; BF::FunctionRef f = s;            EXPECT_EQ(f(), 123); static_assert(std::is_same_v<decltype(f), BF::FunctionRef<int () const noexcept>>); }
	{ RN    s; BF::FunctionRef f = s;            EXPECT_EQ(f(), 123); static_assert(std::is_same_v<decltype(f), BF::FunctionRef<int ()       noexcept>>); }
	{ CRN   s; BF::FunctionRef f = s;            EXPECT_EQ(f(), 123); static_assert(std::is_same_v<decltype(f), BF::FunctionRef<int () const noexcept>>); }
	{ VRN   s; BF::FunctionRef f = s;            EXPECT_EQ(f(), 123); static_assert(std::is_same_v<decltype(f), BF::FunctionRef<int ()       noexcept>>); }
	{ CVRN  s; BF::FunctionRef f = s;            EXPECT_EQ(f(), 123); static_assert(std::is_same_v<decltype(f), BF::FunctionRef<int () const noexcept>>); }
	{ RRN   s; BF::FunctionRef f = std::move(s); EXPECT_EQ(f(), 123); static_assert(std::is_same_v<decltype(f), BF::FunctionRef<int ()       noexcept>>); }
	{ CRRN  s; BF::FunctionRef f = std::move(s); EXPECT_EQ(f(), 123); static_assert(std::is_same_v<decltype(f), BF::FunctionRef<int () const noexcept>>); }
	{ VRRN  s; BF::FunctionRef f = std::move(s); EXPECT_EQ(f(), 123); static_assert(std::is_same_v<decltype(f), BF::FunctionRef<int ()       noexcept>>); }
	{ CVRRN s; BF::FunctionRef f = std::move(s); EXPECT_EQ(f(), 123); static_assert(std::is_same_v<decltype(f), BF::FunctionRef<int () const noexcept>>); }

	struct Zero {};

	struct Two {
		int operator()()       { return 123; }
		int operator()() const { return 123; }
	};

	struct Conv {
		using Fun = void ();
		operator Fun*() const { return nullptr; }
	};

//	{ BF::FunctionRef f = ""; }					// [CompilationError]: 'Type' is not callable. Could not deduce 'Signature' for 'BF::FunctionRef'.
//	{ BF::FunctionRef f = &S::Method; }			// [CompilationError]: 'Type' is not callable. Could not deduce 'Signature' for 'BF::FunctionRef'.
//	{ BF::FunctionRef f = &S::mVar; }			// [CompilationError]: 'Type' is not callable. Could not deduce 'Signature' for 'BF::FunctionRef'.
//	{ BF::FunctionRef f = nullptr; }			// [CompilationError]: 'Type' is not callable. Could not deduce 'Signature' for 'BF::FunctionRef'.
//	{ BF::FunctionRef f = (void*)nullptr; }		// [CompilationError]: 'Type' is not callable. Could not deduce 'Signature' for 'BF::FunctionRef'.
//	{ BF::FunctionRef f = (int*)nullptr; }		// [CompilationError]: 'Type' is not callable. Could not deduce 'Signature' for 'BF::FunctionRef'.
//	{ BF::FunctionRef f = Zero{}; }				// [CompilationError]: 'Type' has zero or more than one operator(). Could not deduce 'Signature' for 'BF::FunctionRef'.
//	{ BF::FunctionRef f = Two{};  }				// [CompilationError]: 'Type' has zero or more than one operator(). Could not deduce 'Signature' for 'BF::FunctionRef'.
//	{ Zero s; BF::FunctionRef f = s; }			// [CompilationError]: 'Type' has zero or more than one operator(). Could not deduce 'Signature' for 'BF::FunctionRef'.
//	{ Two  s; BF::FunctionRef f = s; }			// [CompilationError]: 'Type' has zero or more than one operator(). Could not deduce 'Signature' for 'BF::FunctionRef'.
//	{ Conv s; BF::FunctionRef f = s; }			// [CompilationError]: 'Type' has zero or more than one operator(). Could not deduce 'Signature' for 'BF::FunctionRef'.
}
