#include "BF/TypeTraits.hpp"

#include <cstddef>
#include <vector>
#include "BF/TestUtils.hpp"


// === PrintType =======================================================================================================

// bool b = BF::PrintType<void>;				// [CompilationError]: static_assert failed: 'See the type in the error message.'
// bool b = BF::PrintType<int, int[7]>;			// [CompilationError]: static_assert failed: 'See the type in the error message.'

BF_COMPILE_TIME_TEST()
{
//	BF::PrintType auto printType = 123;			// [CompilationError]: static_assert failed: 'See the type in the error message.'
//	BF::PrintType<void>;						// [CompilationError]: static_assert failed: 'See the type in the error message.'
//	BF::PrintType<int, int[7]>;					// [CompilationError]: static_assert failed: 'See the type in the error message.'
}


// === DontDeduce ======================================================================================================

namespace {


template <class Type>
static void Push(std::vector<Type>& vec, BF::DontDeduce<Type> value)
{
	vec.push_back(std::move(value));
}


struct BigNum {
	BigNum(int) {}
};


}	// namespace


BF_COMPILE_TIME_TEST()
{
	std::vector<BigNum> vec;
	Push(vec, 100);
}


// === Integer =========================================================================================================
// === SInteger ========================================================================================================
// === UInteger ========================================================================================================

namespace {
	struct IntegerSubsumptionChecker {
		static constexpr int Fun(auto)              { return 404; }
		static constexpr int Fun(BF::Integer  auto) { return 200; }
		static constexpr int Fun(BF::SInteger auto) { return  11; }
		static constexpr int Fun(BF::UInteger auto) { return  22; }
	};

	template <BF::Integer  Type> struct IntegerChecker  {};
	template <BF::SInteger Type> struct SIntegerChecker {};
	template <BF::UInteger Type> struct UIntegerChecker {};
}


template <class Type, bool ExpectedResult>
static void TestInteger()
{
	static_assert(BF::IsInteger<Type>                         == ExpectedResult);
	static_assert(requires { typename IntegerChecker<Type>; } == ExpectedResult);
}


template <class Type, bool ExpectedResult>
static void TestSInteger()
{
	static_assert(BF::IsSInteger<Type>                         == ExpectedResult);
	static_assert(requires { typename SIntegerChecker<Type>; } == ExpectedResult);
}


template <class Type, bool ExpectedResult>
static void TestUInteger()
{
	static_assert(BF::IsUInteger<Type>                         == ExpectedResult);
	static_assert(requires { typename UIntegerChecker<Type>; } == ExpectedResult);
}


enum class Signedness {
	Signed,		// Integer, SInteger
	Unsigned,	// Integer, UInteger
	None		// not Integer
};


template <class Type, Signedness S>
static void TestSUN()
{
	TestInteger<Type,                S != Signedness::None>();
	TestInteger<const Type,          S != Signedness::None>();
	TestInteger<volatile Type,       S != Signedness::None>();
	TestInteger<const volatile Type, S != Signedness::None>();

	TestSInteger<Type,                S == Signedness::Signed>();
	TestSInteger<const Type,          S == Signedness::Signed>();
	TestSInteger<volatile Type,       S == Signedness::Signed>();
	TestSInteger<const volatile Type, S == Signedness::Signed>();

	TestUInteger<Type,                S == Signedness::Unsigned>();
	TestUInteger<const Type,          S == Signedness::Unsigned>();
	TestUInteger<volatile Type,       S == Signedness::Unsigned>();
	TestUInteger<const volatile Type, S == Signedness::Unsigned>();
}


BF_COMPILE_TIME_TEST()
{
	static_assert(IntegerSubsumptionChecker::Fun("")    == 404);
	static_assert(IntegerSubsumptionChecker::Fun(false) == 404);
	static_assert(IntegerSubsumptionChecker::Fun(0)     ==  11);
	static_assert(IntegerSubsumptionChecker::Fun(0u)    ==  22);
}


BF_COMPILE_TIME_TEST()
{
	TestSUN<char,               Signedness::Signed>();
	TestSUN<signed char,        Signedness::Signed>();
	TestSUN<unsigned char,      Signedness::Unsigned>();
	TestSUN<char8_t,            Signedness::Unsigned>();
	TestSUN<char16_t,           Signedness::Unsigned>();
	TestSUN<char32_t,           Signedness::Unsigned>();
	TestSUN<wchar_t,            Signedness::Unsigned>();

	TestSUN<short,              Signedness::Signed>();
	TestSUN<int,                Signedness::Signed>();
	TestSUN<long,               Signedness::Signed>();
	TestSUN<long long,          Signedness::Signed>();

	TestSUN<signed short,       Signedness::Signed>();
	TestSUN<signed int,         Signedness::Signed>();
	TestSUN<signed long,        Signedness::Signed>();
	TestSUN<signed long long,   Signedness::Signed>();

	TestSUN<unsigned short,     Signedness::Unsigned>();
	TestSUN<unsigned int,       Signedness::Unsigned>();
	TestSUN<unsigned long,      Signedness::Unsigned>();
	TestSUN<unsigned long long, Signedness::Unsigned>();

	enum E1 {};
	enum class E2 {};
	enum class E3 : int {};

	TestSUN<bool,               Signedness::None>();
	TestSUN<int*,               Signedness::None>();
	TestSUN<int&,               Signedness::None>();
	TestSUN<int&&,              Signedness::None>();
	TestSUN<std::byte,          Signedness::None>();
	TestSUN<std::nullptr_t,     Signedness::None>();
	TestSUN<E1,                 Signedness::None>();
	TestSUN<E2,                 Signedness::None>();
	TestSUN<E3,                 Signedness::None>();
}


// === Decayed =========================================================================================================

namespace {
	template <BF::Decayed Type>
	struct DecayedChecker {};
}


template <class Type, bool ExpectedResult>
static void TestDecayed()
{
	static_assert(BF::IsDecayed<Type>                         == ExpectedResult);
	static_assert(requires { typename DecayedChecker<Type>; } == ExpectedResult);
}


BF_COMPILE_TIME_TEST()
{
	TestDecayed<int,          true >();
	TestDecayed<int*,         true >();
	TestDecayed<const int*,   true >();

	TestDecayed<int&,         false>();
	TestDecayed<int&&,        false>();
	TestDecayed<const int,    false>();
	TestDecayed<volatile int, false>();

	TestDecayed<int[],        false>();
	TestDecayed<int(&)[],     false>();
	TestDecayed<int(&&)[],    false>();
	TestDecayed<int[7],       false>();
	TestDecayed<int(&)[7],    false>();
	TestDecayed<int(&&)[7],   false>();

	TestDecayed<void (),      false>();
	TestDecayed<void (*)(),   true >();
	TestDecayed<void (&)(),   false>();
	TestDecayed<void (&&)(),  false>();
}


// === RelatedTo =======================================================================================================

namespace {
	template <class Type1, BF::RelatedTo<Type1> Type2>
	struct RelatedToChecker {};
}


template <class Type1, class Type2, bool ExpectedResult>
static void TestRelatedTo()
{
	static_assert(BF::AreRelated<Type1, Type2> == ExpectedResult);
	static_assert(BF::AreRelated<Type2, Type1> == ExpectedResult);

	static_assert(requires { typename RelatedToChecker<Type1, Type2>; } == ExpectedResult);
	static_assert(requires { typename RelatedToChecker<Type2, Type1>; } == ExpectedResult);
}


BF_COMPILE_TIME_TEST()
{
	struct Base {};
	struct Derived : Base {};

	TestRelatedTo<Base,  Base,             true >();
	TestRelatedTo<Base,  const Base,       true >();
	TestRelatedTo<Base,  volatile Base,    true >();
	TestRelatedTo<Base,  Derived,          true >();
	TestRelatedTo<Base,  const Derived,    true >();
	TestRelatedTo<Base,  volatile Derived, true >();
	TestRelatedTo<Base,  Base&,            false>();
	TestRelatedTo<Base,  Derived&,         false>();
	TestRelatedTo<Base&, Base&,            false>();
	TestRelatedTo<Base&, Derived&,         false>();
	TestRelatedTo<Base,  void,             false>();
	TestRelatedTo<Base,  std::nullptr_t,   false>();
	TestRelatedTo<Base,  int,              false>();
	TestRelatedTo<void,  void,             false>();
	TestRelatedTo<void*, void*,            false>();
	TestRelatedTo<int,   int,              false>();
}


// === Referenceable ===================================================================================================

template <template <class> class TypeTrait, class Type, bool ExpectedResult>
static void AssertAllCV()
{
	static_assert(TypeTrait<Type>::value                == ExpectedResult);
	static_assert(TypeTrait<const Type>::value          == ExpectedResult);
	static_assert(TypeTrait<volatile Type>::value       == ExpectedResult);
	static_assert(TypeTrait<const volatile Type>::value == ExpectedResult);
}


BF_COMPILE_TIME_TEST()
{
	enum E {};
	enum class EC {};
	class C;
	class CC {};
	union U;
	union UU {};

	AssertAllCV<BF::IsReferenceableT, void,               false>();
	AssertAllCV<BF::IsReferenceableT, std::nullptr_t,     true>();
	AssertAllCV<BF::IsReferenceableT, bool,               true>();
	AssertAllCV<BF::IsReferenceableT, int,                true>();
	AssertAllCV<BF::IsReferenceableT, double,             true>();
	AssertAllCV<BF::IsReferenceableT, char&,              true>();
	AssertAllCV<BF::IsReferenceableT, char&&,             true>();
	AssertAllCV<BF::IsReferenceableT, void*,              true>();
	AssertAllCV<BF::IsReferenceableT, const void*,        true>();
	AssertAllCV<BF::IsReferenceableT, int C::*,           true>();
	AssertAllCV<BF::IsReferenceableT, int C::**,          true>();
	AssertAllCV<BF::IsReferenceableT, int C::*&,          true>();
	AssertAllCV<BF::IsReferenceableT, int C::*&&,         true>();
	AssertAllCV<BF::IsReferenceableT, void (C::*)(),      true>();
	AssertAllCV<BF::IsReferenceableT, void (C::**)(),     true>();
	AssertAllCV<BF::IsReferenceableT, void (C::*&)(),     true>();
	AssertAllCV<BF::IsReferenceableT, void (C::*&&)(),    true>();
	AssertAllCV<BF::IsReferenceableT, void (C::*)() &,    true>();
	AssertAllCV<BF::IsReferenceableT, void (C::**)() &,   true>();
	AssertAllCV<BF::IsReferenceableT, void (C::*&)() &,   true>();
	AssertAllCV<BF::IsReferenceableT, void (C::*&&)() &,  true>();
	AssertAllCV<BF::IsReferenceableT, char[],             true>();
	AssertAllCV<BF::IsReferenceableT, char(*)[],          true>();
	AssertAllCV<BF::IsReferenceableT, char(&)[],          true>();
	AssertAllCV<BF::IsReferenceableT, char(&&)[],         true>();
	AssertAllCV<BF::IsReferenceableT, char[4],            true>();
	AssertAllCV<BF::IsReferenceableT, char(*)[4],         true>();
	AssertAllCV<BF::IsReferenceableT, char(&)[4],         true>();
	AssertAllCV<BF::IsReferenceableT, char(&&)[4],        true>();
	AssertAllCV<BF::IsReferenceableT, void (),            true>();
	AssertAllCV<BF::IsReferenceableT, void (...),         true>();
	AssertAllCV<BF::IsReferenceableT, void () noexcept,   true>();
	AssertAllCV<BF::IsReferenceableT, void (*)(),         true>();
	AssertAllCV<BF::IsReferenceableT, void (&)(),         true>();
	AssertAllCV<BF::IsReferenceableT, void (&&)(),        true>();
	AssertAllCV<BF::IsReferenceableT, void () &,          false>();
	AssertAllCV<BF::IsReferenceableT, void () &&,         false>();
	AssertAllCV<BF::IsReferenceableT, void () const,      false>();
	AssertAllCV<BF::IsReferenceableT, void () const&,     false>();
	AssertAllCV<BF::IsReferenceableT, void () const&&,    false>();
	AssertAllCV<BF::IsReferenceableT, void () volatile,   false>();
	AssertAllCV<BF::IsReferenceableT, void () volatile&,  false>();
	AssertAllCV<BF::IsReferenceableT, void () volatile&&, false>();
	AssertAllCV<BF::IsReferenceableT, E,                  true>();
	AssertAllCV<BF::IsReferenceableT, EC,                 true>();
	AssertAllCV<BF::IsReferenceableT, C,                  true>();
	AssertAllCV<BF::IsReferenceableT, CC,                 true>();
	AssertAllCV<BF::IsReferenceableT, U,                  true>();
	AssertAllCV<BF::IsReferenceableT, UU,                 true>();

	BF::Referenceable auto x = 1;
}


// === Abominable ======================================================================================================

template <BF::Abominable Type>
static void TestAbominable() {};


BF_COMPILE_TIME_TEST()
{
	enum E {};
	enum class EC {};
	class C;
	class CC {};
	union U;
	union UU {};

	AssertAllCV<BF::IsAbominableT, void,               false>();
	AssertAllCV<BF::IsAbominableT, std::nullptr_t,     false>();
	AssertAllCV<BF::IsAbominableT, bool,               false>();
	AssertAllCV<BF::IsAbominableT, int,                false>();
	AssertAllCV<BF::IsAbominableT, double,             false>();
	AssertAllCV<BF::IsAbominableT, char&,              false>();
	AssertAllCV<BF::IsAbominableT, char&&,             false>();
	AssertAllCV<BF::IsAbominableT, void*,              false>();
	AssertAllCV<BF::IsAbominableT, const void*,        false>();
	AssertAllCV<BF::IsAbominableT, int C::*,           false>();
	AssertAllCV<BF::IsAbominableT, int C::**,          false>();
	AssertAllCV<BF::IsAbominableT, int C::*&,          false>();
	AssertAllCV<BF::IsAbominableT, int C::*&&,         false>();
	AssertAllCV<BF::IsAbominableT, void (C::*)(),      false>();
	AssertAllCV<BF::IsAbominableT, void (C::**)(),     false>();
	AssertAllCV<BF::IsAbominableT, void (C::*&)(),     false>();
	AssertAllCV<BF::IsAbominableT, void (C::*&&)(),    false>();
	AssertAllCV<BF::IsAbominableT, void (C::*)() &,    false>();
	AssertAllCV<BF::IsAbominableT, void (C::**)() &,   false>();
	AssertAllCV<BF::IsAbominableT, void (C::*&)() &,   false>();
	AssertAllCV<BF::IsAbominableT, void (C::*&&)() &,  false>();
	AssertAllCV<BF::IsAbominableT, char[],             false>();
	AssertAllCV<BF::IsAbominableT, char(*)[],          false>();
	AssertAllCV<BF::IsAbominableT, char(&)[],          false>();
	AssertAllCV<BF::IsAbominableT, char(&&)[],         false>();
	AssertAllCV<BF::IsAbominableT, char[4],            false>();
	AssertAllCV<BF::IsAbominableT, char(*)[4],         false>();
	AssertAllCV<BF::IsAbominableT, char(&)[4],         false>();
	AssertAllCV<BF::IsAbominableT, char(&&)[4],        false>();
	AssertAllCV<BF::IsAbominableT, void (),            false>();
	AssertAllCV<BF::IsAbominableT, void (...),         false>();
	AssertAllCV<BF::IsAbominableT, void () noexcept,   false>();
	AssertAllCV<BF::IsAbominableT, void (*)(),         false>();
	AssertAllCV<BF::IsAbominableT, void (&)(),         false>();
	AssertAllCV<BF::IsAbominableT, void (&&)(),        false>();
	AssertAllCV<BF::IsAbominableT, void () &,          true>();
	AssertAllCV<BF::IsAbominableT, void () &&,         true>();
	AssertAllCV<BF::IsAbominableT, void () const,      true>();
	AssertAllCV<BF::IsAbominableT, void () const&,     true>();
	AssertAllCV<BF::IsAbominableT, void () const&&,    true>();
	AssertAllCV<BF::IsAbominableT, void () volatile,   true>();
	AssertAllCV<BF::IsAbominableT, void () volatile&,  true>();
	AssertAllCV<BF::IsAbominableT, void () volatile&&, true>();
	AssertAllCV<BF::IsAbominableT, E,                  false>();
	AssertAllCV<BF::IsAbominableT, EC,                 false>();
	AssertAllCV<BF::IsAbominableT, C,                  false>();
	AssertAllCV<BF::IsAbominableT, CC,                 false>();
	AssertAllCV<BF::IsAbominableT, U,                  false>();
	AssertAllCV<BF::IsAbominableT, UU,                 false>();

	TestAbominable<void () &>();
}


// === AreConstRelated =================================================================================================

static_assert( BF::AreConstRelated<void,                 void>);
static_assert( BF::AreConstRelated<void,                 const void>);
static_assert(!BF::AreConstRelated<void,                 volatile void>);
static_assert(!BF::AreConstRelated<void,                 const volatile void>);
static_assert( BF::AreConstRelated<const void,           void>);
static_assert( BF::AreConstRelated<const void,           const void>);
static_assert(!BF::AreConstRelated<const void,           volatile void>);
static_assert(!BF::AreConstRelated<const void,           const volatile void>);
static_assert(!BF::AreConstRelated<volatile void,        void>);
static_assert(!BF::AreConstRelated<volatile void,        const void>);
static_assert( BF::AreConstRelated<volatile void,        volatile void>);
static_assert( BF::AreConstRelated<volatile void,        const volatile void>);
static_assert(!BF::AreConstRelated<const volatile void,  void>);
static_assert(!BF::AreConstRelated<const volatile void,  const void>);
static_assert( BF::AreConstRelated<const volatile void,  volatile void>);
static_assert( BF::AreConstRelated<const volatile void,  const volatile void>);

static_assert( BF::AreConstRelated<void*,                const void*>);
static_assert( BF::AreConstRelated<void*,                const void* const>);
static_assert(!BF::AreConstRelated<void*,                const void* volatile>);
static_assert(!BF::AreConstRelated<void*,                const void* const volatile>);
static_assert(!BF::AreConstRelated<void* volatile,       const void*>);
static_assert(!BF::AreConstRelated<void* volatile,       const void* const>);
static_assert( BF::AreConstRelated<void* volatile,       const void* volatile>);
static_assert( BF::AreConstRelated<void* volatile,       const void* const volatile>);

static_assert( BF::AreConstRelated<int*,                 const int*>);
static_assert( BF::AreConstRelated<int&,                 const int&>);
static_assert( BF::AreConstRelated<int&&,                const int&&>);
static_assert(!BF::AreConstRelated<int,                  int&>);
static_assert(!BF::AreConstRelated<int,                  int&&>);
static_assert(!BF::AreConstRelated<int*,                 int**>);
static_assert(!BF::AreConstRelated<int&,                 int&&>);

static_assert( BF::AreConstRelated<int**,                int const* const* const>);
static_assert(!BF::AreConstRelated<int**,                int volatile* const* const>);
static_assert(!BF::AreConstRelated<int**,                int const* volatile* const>);
static_assert(!BF::AreConstRelated<int**,                int const* const* volatile>);
static_assert( BF::AreConstRelated<int**&,               int const* const* const&>);


// === NotSelf =========================================================================================================

namespace {


struct Base {
	Base() = default;
	Base(BF::NotSelf<Base> auto&& x)			{ static_assert(false); }
	Base& operator=(BF::NotSelf<Base> auto&& x)	{ static_assert(false); }
};


struct Derived : Base {};


}	// namespace


BF_COMPILE_TIME_TEST()
[[gsl::suppress(es.56)]]		// allow std::move on constant variable
{
	{ Base          s; Base    t = s;            }		// without BF::NotSelf this calls the auto&& ctor.
	{ Base          s; Base    t = std::move(s); }		// exactly matches the move ctor.
	{ const Base    s; Base    t = s;            }		// exactly matches the copy ctor.
	{ const Base    s; Base    t = std::move(s); }		// without BF::NotSelf this calls the auto&& ctor.
	{ Derived       s; Base    t = s;            }		// without BF::NotSelf this calls the auto&& ctor.
	{ Derived       s; Base    t = std::move(s); }		// without BF::NotSelf this calls the auto&& ctor.
	{ const Derived s; Base    t = s;            }		// without BF::NotSelf this calls the auto&& ctor.
	{ const Derived s; Base    t = std::move(s); }		// without BF::NotSelf this calls the auto&& ctor.

	{ Base          s; Base t; t = s;            }		// without BF::NotSelf this calls the auto&& assignment
	{ Base          s; Base t; t = std::move(s); }		// exactly matches the move assignment
	{ const Base    s; Base t; t = s;            }		// exactly matches the copy assignment
	{ const Base    s; Base t; t = std::move(s); }		// without BF::NotSelf this calls the auto&& assignment
	{ Derived       s; Base t; t = s;            }		// without BF::NotSelf this calls the auto&& assignment
	{ Derived       s; Base t; t = std::move(s); }		// without BF::NotSelf this calls the auto&& assignment
	{ const Derived s; Base t; t = s;            }		// without BF::NotSelf this calls the auto&& assignment
	{ const Derived s; Base t; t = std::move(s); }		// without BF::NotSelf this calls the auto&& assignment
}


// === AddConstBeforeRef, AddVolatileBeforeRef, AddConstVolatileBeforeRef ==============================================

template <template <class> class TypeTrait, class Type, class ExpectedType>
static void TestAddBeforeRef()
{
	static_assert(!std::is_reference_v<Type>);
	static_assert(!std::is_reference_v<ExpectedType>);

	static_assert(std::is_same_v<typename TypeTrait<Type>::type,   ExpectedType>);
	static_assert(std::is_same_v<typename TypeTrait<Type&>::type,  ExpectedType&>);
	static_assert(std::is_same_v<typename TypeTrait<Type&&>::type, ExpectedType&&>);
}


BF_COMPILE_TIME_TEST()
{
	TestAddBeforeRef<BF::AddConstBeforeRefT,         int,                const int         >();
	TestAddBeforeRef<BF::AddConstBeforeRefT,         const int,          const int         >();
	TestAddBeforeRef<BF::AddConstBeforeRefT,         volatile int,       const volatile int>();
	TestAddBeforeRef<BF::AddConstBeforeRefT,         const volatile int, const volatile int>();

	TestAddBeforeRef<BF::AddVolatileBeforeRefT,      int,                volatile int      >();
	TestAddBeforeRef<BF::AddVolatileBeforeRefT,      const int,          const volatile int>();
	TestAddBeforeRef<BF::AddVolatileBeforeRefT,      volatile int,       volatile int      >();
	TestAddBeforeRef<BF::AddVolatileBeforeRefT,      const volatile int, const volatile int>();

	TestAddBeforeRef<BF::AddConstVolatileBeforeRefT, int,                const volatile int>();
	TestAddBeforeRef<BF::AddConstVolatileBeforeRefT, const int,          const volatile int>();
	TestAddBeforeRef<BF::AddConstVolatileBeforeRefT, volatile int,       const volatile int>();
	TestAddBeforeRef<BF::AddConstVolatileBeforeRefT, const volatile int, const volatile int>();

	static_assert(std::is_same_v<BF::AddConstBeforeRef<int>,          const int>);
	static_assert(std::is_same_v<BF::AddVolatileBeforeRef<int>,       volatile int>);
	static_assert(std::is_same_v<BF::AddConstVolatileBeforeRef<int>,  const volatile int>);

	static_assert(std::is_same_v<BF::AddConstBeforeRef<int*>,         int* const>);
	static_assert(std::is_same_v<BF::AddVolatileBeforeRef<int*>,      int* volatile>);
	static_assert(std::is_same_v<BF::AddConstVolatileBeforeRef<int*>, int* const volatile>);
}


// === RemoveConstBeforeRef, RemoveVolatileBeforeRef, RemoveConstVolatileBeforeRef =====================================

template <template <class> class TypeTrait, class Type, class ExpectedType>
static void TestRemoveBeforeRef()
{
	static_assert(!std::is_reference_v<Type>);
	static_assert(!std::is_reference_v<ExpectedType>);

	static_assert(std::is_same_v<typename TypeTrait<Type>::type,   ExpectedType>);
	static_assert(std::is_same_v<typename TypeTrait<Type&>::type,  ExpectedType&>);
	static_assert(std::is_same_v<typename TypeTrait<Type&&>::type, ExpectedType&&>);
}


BF_COMPILE_TIME_TEST()
{
	TestRemoveBeforeRef<BF::RemoveConstBeforeRefT,         int,                int         >();
	TestRemoveBeforeRef<BF::RemoveConstBeforeRefT,         const int,          int         >();
	TestRemoveBeforeRef<BF::RemoveConstBeforeRefT,         volatile int,       volatile int>();
	TestRemoveBeforeRef<BF::RemoveConstBeforeRefT,         const volatile int, volatile int>();

	TestRemoveBeforeRef<BF::RemoveVolatileBeforeRefT,      int,                int      >();
	TestRemoveBeforeRef<BF::RemoveVolatileBeforeRefT,      const int,          const int>();
	TestRemoveBeforeRef<BF::RemoveVolatileBeforeRefT,      volatile int,       int      >();
	TestRemoveBeforeRef<BF::RemoveVolatileBeforeRefT,      const volatile int, const int>();

	TestRemoveBeforeRef<BF::RemoveConstVolatileBeforeRefT, int,                int>();
	TestRemoveBeforeRef<BF::RemoveConstVolatileBeforeRefT, const int,          int>();
	TestRemoveBeforeRef<BF::RemoveConstVolatileBeforeRefT, volatile int,       int>();
	TestRemoveBeforeRef<BF::RemoveConstVolatileBeforeRefT, const volatile int, int>();

	static_assert(std::is_same_v<BF::RemoveConstBeforeRef<const volatile int>,          volatile int>);
	static_assert(std::is_same_v<BF::RemoveVolatileBeforeRef<const volatile int>,       const int>);
	static_assert(std::is_same_v<BF::RemoveConstVolatileBeforeRef<const volatile int>,  int>);

	static_assert(std::is_same_v<BF::RemoveConstBeforeRef<const volatile int*>,         const volatile int*>);
	static_assert(std::is_same_v<BF::RemoveVolatileBeforeRef<const volatile int*>,      const volatile int*>);
	static_assert(std::is_same_v<BF::RemoveConstVolatileBeforeRef<const volatile int*>, const volatile int*>);
}


// === std26::copy_cvref_t =============================================================================================

template <class From, class To>
static void TestCopyCVRef()
{
	static_assert(!std::is_reference_v<From>);
	static_assert(!std::is_reference_v<To>);

	using CTo  = std::conditional_t<std::is_const_v<From>,    const To,     To>;
	using CVTo = std::conditional_t<std::is_volatile_v<From>, volatile CTo, CTo>;

	static_assert(std::is_same_v<BF::std26::copy_cvref_t<From,   To>,   CVTo>);
	static_assert(std::is_same_v<BF::std26::copy_cvref_t<From&,  To>,   CVTo&>);
	static_assert(std::is_same_v<BF::std26::copy_cvref_t<From&&, To>,   CVTo&&>);

	static_assert(std::is_same_v<BF::std26::copy_cvref_t<From,   To&>,  CVTo&>);
	static_assert(std::is_same_v<BF::std26::copy_cvref_t<From&,  To&>,  CVTo&>);
	static_assert(std::is_same_v<BF::std26::copy_cvref_t<From&&, To&>,  CVTo&>);

	static_assert(std::is_same_v<BF::std26::copy_cvref_t<From,   To&&>, CVTo&&>);
	static_assert(std::is_same_v<BF::std26::copy_cvref_t<From&,  To&&>, CVTo&>);
	static_assert(std::is_same_v<BF::std26::copy_cvref_t<From&&, To&&>, CVTo&&>);

}


BF_COMPILE_TIME_TEST()
{
	TestCopyCVRef<int,                char>();
	TestCopyCVRef<const int,          char>();
	TestCopyCVRef<volatile int,       char>();
	TestCopyCVRef<const volatile int, char>();

	TestCopyCVRef<int,                const char>();
	TestCopyCVRef<const int,          const char>();
	TestCopyCVRef<volatile int,       const char>();
	TestCopyCVRef<const volatile int, const char>();

	TestCopyCVRef<int,                volatile char>();
	TestCopyCVRef<const int,          volatile char>();
	TestCopyCVRef<volatile int,       volatile char>();
	TestCopyCVRef<const volatile int, volatile char>();

	TestCopyCVRef<int,                const volatile char>();
	TestCopyCVRef<const int,          const volatile char>();
	TestCopyCVRef<volatile int,       const volatile char>();
	TestCopyCVRef<const volatile int, const volatile char>();
}
