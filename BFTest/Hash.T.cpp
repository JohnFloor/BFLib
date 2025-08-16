#include "BF/Hash.hpp"

#include <unordered_set>
#include "BF/TestUtils.hpp"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////// Helpers /////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


namespace {


// === TRY_HASH_CONTAINER ==============================================================================================

#define TRY_HASH_CONTAINER(Type)	std::unordered_set<Type> BF_DUMMY = { Type(), Type(), Type() }


// === enum MyEnum =====================================================================================================

enum MyEnum {};


// === AssertIsHashable() ==============================================================================================

enum HashableCheckMethod {
	CheckBFAndStd,
	CheckOnlyStd
};


template <class T, HashableCheckMethod Method = CheckBFAndStd>
void AssertIsHashable()
{
	static_assert(std::equality_comparable<T>);
	static_assert((Method == CheckBFAndStd) BF_IMPLIES BF::HasGetHash<T>);
	static_assert(BF::StdHashable<T>);
	TRY_HASH_CONTAINER(T);
}


// === struct ConstexprHashable ========================================================================================

struct ConstexprHashable {
	std::size_t size;
};


}	// namespace


template <>										// this must be in file scope, outside namespace {}
struct std::hash<ConstexprHashable> {
	[[nodiscard]]
	constexpr static std::size_t operator()(ConstexprHashable value) { return value.size; }
};


namespace {


constexpr std::size_t BF_DUMMY = std::hash<ConstexprHashable>()({});			// try out compile time evaluation


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////// Test cases //////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// === BF::Hash ctor. ==================================================================================================

struct EmptyHash {
//	BF::Hash BF_GetHash() const { return {}; }					// [CompilationError]: BF::Hash: Provide at least one value to hash.
};


struct Meth1 {
	bool operator==(const Meth1&) const = default;
	BF::Hash BF_GetHash() const { return { m1 }; }
	UInt64 m1;
};


struct Meth2 {
	bool operator==(const Meth2&) const = default;
	BF::Hash BF_GetHash() const { return { m1, m2 }; }
	MyEnum m1;
	UInt16 m2;
};


struct Fun1 {
	bool operator==(const Fun1&) const = default;
	UInt64 m1;
};
BF::Hash BF_GetHash(const Fun1& value) { return { value.m1 }; }


struct Fun2 {
	bool operator==(const Fun2&) const = default;
	MyEnum m1;
	UInt16 m2;
};
BF::Hash BF_GetHash(const Fun2& value) { return { value.m1, value.m2 }; }


struct WrappedM {
	bool operator==(const WrappedM&) const = default;
	BF::Hash BF_GetHash() const { return { m1 }; }
	Meth1 m1;
};


struct WrappedF {
	bool operator==(const WrappedF&) const = default;
	BF::Hash BF_GetHash() const { return { m1 }; }
	Fun1  m1;
};


struct Wrapped3 {
	bool operator==(const Wrapped3&) const = default;
	BF::Hash BF_GetHash() const { return { m1, m2, m3 }; }
	Meth1 m1;
	Fun1  m2;
	bool  m3;
};


BF_COMPILE_TIME_TEST() { AssertIsHashable<Meth1>(); }
BF_COMPILE_TIME_TEST() { AssertIsHashable<Meth2>(); }
BF_COMPILE_TIME_TEST() { AssertIsHashable<Fun1>(); }
BF_COMPILE_TIME_TEST() { AssertIsHashable<Fun2>(); }
BF_COMPILE_TIME_TEST() { AssertIsHashable<WrappedM>(); }
BF_COMPILE_TIME_TEST() { AssertIsHashable<WrappedF>(); }
BF_COMPILE_TIME_TEST() { AssertIsHashable<Wrapped3>(); }


// === BF::Hash ctor.: constructing from a not hashable type ===========================================================

struct NotHashable {};


struct FromNotHashable1 {
//	BF::Hash BF_GetHash() const { return { m1 }; }				// [CompilationError]: 'Type' is not hashable.
	NotHashable m1;
};


struct FromNotHashable2 {
//	BF::Hash BF_GetHash() const { return { m1, m2 }; }			// [CompilationError]: 'Type' is not hashable.
	UInt64      m1;
	NotHashable m2;
};


// === BF::HasGetHash: detecting BF_GetHash() method ===================================================================

#define TEST_METHOD_DETECTION(...)					\
namespace BF_DUMMY_NAME {							\
	struct H { void BF_GetHash() __VA_ARGS__; };	\
													\
	class DPub  : public H {};						\
	class DProt : protected H {};					\
	class DPri  : private H {};						\
	struct Conv1 { operator H(); };					\
	struct Conv2 { operator const H(); };			\
	struct Conv3 { operator volatile H(); };		\
	struct Conv4 { operator const volatile H(); };	\
	struct Conv5 { operator H&(); };				\
	struct Conv6 { operator const H&(); };			\
	struct Conv7 { operator volatile H&(); };		\
	struct Conv8 { operator const volatile H&(); };	\
													\
	static_assert( BF::HasGetHash<H>);				\
	static_assert( BF::HasGetHash<DPub>);  /*!!!*/	\
	static_assert(!BF::HasGetHash<DProt>);			\
	static_assert(!BF::HasGetHash<DPri>);			\
	static_assert(!BF::HasGetHash<Conv1>);			\
	static_assert(!BF::HasGetHash<Conv2>);			\
	static_assert(!BF::HasGetHash<Conv3>);			\
	static_assert(!BF::HasGetHash<Conv4>);			\
	static_assert(!BF::HasGetHash<Conv5>);			\
	static_assert(!BF::HasGetHash<Conv6>);			\
	static_assert(!BF::HasGetHash<Conv7>);			\
	static_assert(!BF::HasGetHash<Conv8>);			\
}


TEST_METHOD_DETECTION()
TEST_METHOD_DETECTION(const)
TEST_METHOD_DETECTION(volatile)
TEST_METHOD_DETECTION(const volatile)
TEST_METHOD_DETECTION(&)
TEST_METHOD_DETECTION(const&)
TEST_METHOD_DETECTION(volatile&)
TEST_METHOD_DETECTION(const volatile&)
TEST_METHOD_DETECTION(&&)
TEST_METHOD_DETECTION(const&&)
TEST_METHOD_DETECTION(volatile&&)
TEST_METHOD_DETECTION(const volatile&&)


// === BF::HasGetHash: detecting BF_GetHash() function =================================================================

#define TEST_FUNCTION_DETECTION(...)				\
namespace BF_DUMMY_NAME {							\
	struct H {};									\
	void BF_GetHash(H __VA_ARGS__);					\
													\
	class DPub  : public H {};						\
	class DProt : protected H {};					\
	class DPri  : private H {};						\
	struct Conv1 { operator H(); };					\
	struct Conv2 { operator const H(); };			\
	struct Conv3 { operator volatile H(); };		\
	struct Conv4 { operator const volatile H(); };	\
	struct Conv5 { operator H&(); };				\
	struct Conv6 { operator const H&(); };			\
	struct Conv7 { operator volatile H&(); };		\
	struct Conv8 { operator const volatile H&(); };	\
													\
	static_assert( BF::HasGetHash<H>);				\
	static_assert(!BF::HasGetHash<DPub>);			\
	static_assert(!BF::HasGetHash<DProt>);			\
	static_assert(!BF::HasGetHash<DPri>);			\
	static_assert(!BF::HasGetHash<Conv1>);			\
	static_assert(!BF::HasGetHash<Conv2>);			\
	static_assert(!BF::HasGetHash<Conv3>);			\
	static_assert(!BF::HasGetHash<Conv4>);			\
	static_assert(!BF::HasGetHash<Conv5>);			\
	static_assert(!BF::HasGetHash<Conv6>);			\
	static_assert(!BF::HasGetHash<Conv7>);			\
	static_assert(!BF::HasGetHash<Conv8>);			\
}


TEST_FUNCTION_DETECTION()
TEST_FUNCTION_DETECTION(const)
TEST_FUNCTION_DETECTION(volatile)
TEST_FUNCTION_DETECTION(const volatile)
TEST_FUNCTION_DETECTION(&)
TEST_FUNCTION_DETECTION(const&)
TEST_FUNCTION_DETECTION(volatile&)
TEST_FUNCTION_DETECTION(const volatile&)
TEST_FUNCTION_DETECTION(&&)
TEST_FUNCTION_DETECTION(const&&)
TEST_FUNCTION_DETECTION(volatile&&)
TEST_FUNCTION_DETECTION(const volatile&&)


// === BF::HasGetHash: having more classes with BF_GetHash() function in the same namespace ============================

class Over1 {};
bool operator==(const Over1&, const Over1&) { return true; }
BF::Hash BF_GetHash(const Over1&) { return { 0 }; }

class Over2 {};
bool operator==(const Over2&, const Over2&) { return true; }
BF::Hash BF_GetHash(const Over2&) { return { 0 }; }


BF_COMPILE_TIME_TEST() { AssertIsHashable<Over1>(); }
BF_COMPILE_TIME_TEST() { AssertIsHashable<Over2>(); }


// === Not hashable ====================================================================================================

struct NoGetHash {};


class Private {
	BF::Hash BF_GetHash() const { return { 0 }; }
};


namespace N {
	struct BadNS {};
}
BF::Hash BF_GetHash(const N::BadNS&) { return { 0 }; }


static_assert(!BF::HasGetHash<NoGetHash>);
static_assert(!BF::StdHashable<NoGetHash>);
// TRY_HASH_CONTAINER(NoGetHash);								// [CompilationError]: hash(void)': attempting to reference a deleted function

static_assert(!BF::HasGetHash<Private>);
static_assert(!BF::StdHashable<Private>);
// TRY_HASH_CONTAINER(Private);									// [CompilationError]: hash(void)': attempting to reference a deleted function

static_assert(!BF::HasGetHash<N::BadNS>);
static_assert(!BF::StdHashable<N::BadNS>);
// TRY_HASH_CONTAINER(N::BadNS);								// [CompilationError]: hash(void)': attempting to reference a deleted function


// === Ambiguous hashing ===============================================================================================

struct Ambiguous {
	BF::Hash BF_GetHash() const { return { 0 }; }
};
BF::Hash BF_GetHash(const Ambiguous&) { return { 0 }; }


static_assert(BF::HasGetHash<Ambiguous> );
// static_assert(BF::StdHashable<Ambiguous>);					// [CompilationError]: Ambiguous hashing
// BF::Hash BF_DUMMY = { Ambiguous() };							// [CompilationError]: Ambiguous hashing
// TRY_HASH_CONTAINER(Ambiguous);								// [CompilationError]: Ambiguous hashing


// === Bad signature for BF_GetHash() ==================================================================================

struct BadConstM {
	BF::Hash BF_GetHash() /*const*/ { return { 0 }; }
};


struct BadConstF {};
BF::Hash BF_GetHash(/*const*/ BadConstF&) { return { 0 }; }


struct BadReturnM {
	const BF::Hash BF_GetHash() const { return { 0 }; }
};


struct BadReturnF {};
const BF::Hash BF_GetHash(const BadReturnF&) { return { 0 }; }


struct BadReturnMs {
	std::size_t BF_GetHash() const { return 0; }
};


struct BadReturnFs {};
std::size_t BF_GetHash(const BadReturnFs&) { return 0; }


static_assert(BF::HasGetHash<BadConstM>);
// static_assert(BF::StdHashable<BadConstM>);					// [CompilationError]: BF_GetHash() method should be const.
// BF::Hash BF_DUMMY = { BadConstM() };							// [CompilationError]: BF_GetHash() method should be const.
// TRY_HASH_CONTAINER(BadConstM);								// [CompilationError]: BF_GetHash() method should be const.

static_assert(BF::HasGetHash<BadConstF>);
// static_assert(BF::StdHashable<BadConstF>);					// [CompilationError]: BF_GetHash() function's parameter should be const.
// BF::Hash BF_DUMMY = { BadConstF() };							// [CompilationError]: BF_GetHash() function's parameter should be const.
// TRY_HASH_CONTAINER(BadConstF);								// [CompilationError]: BF_GetHash() function's parameter should be const.

static_assert(BF::HasGetHash<BadReturnM>);
// static_assert(BF::StdHashable<BadReturnM>);					// [CompilationError]: BF_GetHash() method should return BF::Hash.
// BF::Hash BF_DUMMY = { BadReturnM() };						// [CompilationError]: BF_GetHash() method should return BF::Hash.
// TRY_HASH_CONTAINER(BadReturnM);								// [CompilationError]: BF_GetHash() method should return BF::Hash.

static_assert(BF::HasGetHash<BadReturnF>);
// static_assert(BF::StdHashable<BadReturnF>);					// [CompilationError]: BF_GetHash() function should return BF::Hash.
// BF::Hash BF_DUMMY = { BadReturnF() };						// [CompilationError]: BF_GetHash() function should return BF::Hash.
// TRY_HASH_CONTAINER(BadReturnF);								// [CompilationError]: BF_GetHash() function should return BF::Hash.

static_assert(BF::HasGetHash<BadReturnMs>);
// static_assert(BF::StdHashable<BadReturnMs>);					// [CompilationError]: BF_GetHash() method should return BF::Hash.
// BF::Hash BF_DUMMY = { BadReturnMs() };						// [CompilationError]: BF_GetHash() method should return BF::Hash.
// TRY_HASH_CONTAINER(BadReturnMs);								// [CompilationError]: BF_GetHash() method should return BF::Hash.

static_assert(BF::HasGetHash<BadReturnFs>);
// static_assert(BF::StdHashable<BadReturnFs>);					// [CompilationError]: BF_GetHash() function should return BF::Hash.
// BF::Hash BF_DUMMY = { BadReturnFs() };						// [CompilationError]: BF_GetHash() function should return BF::Hash.
// TRY_HASH_CONTAINER(BadReturnFs);								// [CompilationError]: BF_GetHash() function should return BF::Hash.


// === Deriving from a class with BF_GetHash() method: the derived is also hashable ====================================

struct BaseM {
	bool operator==(const BaseM&) const = default;
	BF::Hash BF_GetHash() const { return { 0 }; }
};


struct DerivedM : BaseM {};


BF_COMPILE_TIME_TEST() { AssertIsHashable<BaseM>(); }
BF_COMPILE_TIME_TEST() { AssertIsHashable<DerivedM>(); }


// === Deriving from a class with BF_GetHash() method: disabling hashing in the derived ================================

struct DerivedM2 : BaseM {
	BF::Hash BF_GetHash() const = delete;						// you have to define a deleted method, and not a function
};


static_assert(std::equality_comparable<DerivedM2>);
static_assert(!BF::HasGetHash<DerivedM2>);
static_assert(!BF::StdHashable<DerivedM2>);
// TRY_HASH_CONTAINER(DerivedM2);								// [CompilationError]: hash(void)': attempting to reference a deleted function


// === Deriving from a class with BF_GetHash() function: the derived is not hashable ===================================

struct BaseF {};
bool operator==(const BaseF&, const BaseF&) { return true; }
BF::Hash BF_GetHash(const BaseF&) { return { 0 }; }


struct DerivedF : BaseF {};


BF_COMPILE_TIME_TEST() { AssertIsHashable<BaseF>(); }

static_assert(std::equality_comparable<DerivedF>);
static_assert(!BF::HasGetHash<DerivedF>);
static_assert(!BF::StdHashable<DerivedF>);
// TRY_HASH_CONTAINER(DerivedF);								// [CompilationError]: hash(void)': attempting to reference a deleted function


// === Class that is convertible to a class with BF_GetHash() method: it's not hashable  ===============================

struct SourceM {};


struct TargetM {
	TargetM() {}
	TargetM(const SourceM&) {}
	bool operator==(const TargetM&) const = default;
	BF::Hash BF_GetHash() const { return { 0 }; }
};


static_assert(!std::equality_comparable<SourceM>);
static_assert(!BF::HasGetHash<SourceM>);
static_assert(!BF::StdHashable<SourceM>);
// TRY_HASH_CONTAINER(SourceM);									// [CompilationError]: hash(void)': attempting to reference a deleted function

BF_COMPILE_TIME_TEST() { AssertIsHashable<TargetM>(); }


// === Class that is convertible to a class with BF_GetHash() function: it's not hashable  =============================

struct SourceF {};


struct TargetF {
	TargetF() {}
	TargetF(const SourceF&) {}
};
bool operator==(const TargetF&, const TargetF&) { return true; }
BF::Hash BF_GetHash(const TargetF&) { return { 0 }; }


static_assert(std::equality_comparable<SourceF>);
static_assert(!BF::HasGetHash<SourceF>);
static_assert(!BF::StdHashable<SourceF>);
// TRY_HASH_CONTAINER(SourceF);									// [CompilationError]: hash(void)': attempting to reference a deleted function

BF_COMPILE_TIME_TEST() { AssertIsHashable<TargetF>(); }


// === Class that is convertible to a type hashable by 'std::hash': it's not hashable  =================================

struct ConvInt {
	operator int() const { return 0; }
};


static_assert(std::equality_comparable<ConvInt>);
static_assert(!BF::HasGetHash<ConvInt>);
static_assert(!BF::StdHashable<ConvInt>);
// TRY_HASH_CONTAINER(ConvInt);									// [CompilationError]: hash(void)': attempting to reference a deleted function


// === BF::Hash: final =================================================================================================

static_assert(std::is_final_v<BF::Hash>);


// === BF::Hash: constexpr =============================================================================================

struct ConstexprM {
	constexpr BF::Hash BF_GetHash() const { return { m1, m2 }; }		// two values => hash combination is used
	ConstexprHashable m1;
	ConstexprHashable m2;
};


struct ConstexprF {
	ConstexprHashable m;
};
constexpr BF::Hash BF_GetHash(const ConstexprF& value) { return { value.m }; }


constexpr std::size_t BF_DUMMY = std::hash<ConstexprM>()({});
constexpr std::size_t BF_DUMMY = std::hash<ConstexprF>()({});


// BF::Hash: outside std::hash it is not convertible to std::size_t ====================================================

// std::size_t BF_DUMMY = BF::Hash(0);							// [CompilationError]: 'BF::Hash::operator size_t': cannot access private member
// std::size_t BF_DUMMY(BF::Hash(0));							// [CompilationError]: 'BF::Hash::operator size_t': cannot access private member


// === Hashing a class template of a 3rd party library =================================================================
// There are four roles in this example:
//   - Role0: The author of library 'BF'.
//   - Role1: The author of the 3rd party library 'Lib'.
//   - Role2: C++ infrastructure developer of company XYZ. Client of Role0 and Role1. Makes 'BF' and 'Lib' available to
//            all application developers. Commits them to XYZ's repository. Includes them in the build. Updates them, if
//            necessary. And, makes some types of 'Lib' hashable with the help of 'BF'.
//   - Role3: Application developer of company XYZ. Client of Role2. He won't directly #include "Lib/Counted.hpp".
//            Since this header has an adaptation in XYZ, he is only allowed to #include "Adapted/Lib/Counted.hpp".

// === Contents of "Lib/Counted.hpp" (author: Role1):
/* #pragma once */
namespace Lib {
	template <class Type>
	struct Counted {
		Type     type{};
		unsigned count{};
	};
}


// === Contents of "Adapted/Lib/Counted.hpp" (author: Role2):
/* #pragma once */
/* #include <type_traits>     */
/* #include "BF/Hash.hpp"     */
/* #include "Lib/Counted.hpp" */
namespace Lib {
	template <std::equality_comparable Type>
	bool operator==(const Counted<Type>& leftOp, const Counted<Type>& rightOp)
	{
		return leftOp.type == rightOp.type && leftOp.count == rightOp.count;
	}
}

template <BF::StdHashable Type>
struct std::hash<Lib::Counted<Type>> {
	[[nodiscard]]
	static std::size_t operator()(const Lib::Counted<Type>& value) {
		return BF::Hash(value.type, value.count);				// 'BF::Hash' will convert to 'std::size_t' inside 'std::hash' specializations
	}
};


// === Source file (author: Role3):
/* #include "Adapted/Lib/Counted.hpp" */
BF_COMPILE_TIME_TEST() { AssertIsHashable<Lib::Counted<int>,   CheckOnlyStd>(); }
BF_COMPILE_TIME_TEST() { AssertIsHashable<Lib::Counted<Meth1>, CheckOnlyStd>(); }
BF_COMPILE_TIME_TEST() { AssertIsHashable<Lib::Counted<Fun1>,  CheckOnlyStd>(); }

static_assert(!BF::StdHashable<Lib::Counted<NotHashable>>);
// TRY_HASH_CONTAINER(Lib::Counted<NotHashable>);				// [CompilationError]: hash(void)': attempting to reference a deleted function


}	// namespace
