// Utilities for writing tests.


#pragma once
#include <new>
#include <type_traits>
#include "BF/ClassUtils.hpp"
#include "BF/RawMemory.hpp"


// === BF_COMPILE_TIME_TEST ============================================================================================

#define BF_COMPILE_TIME_TEST							\
[[maybe_unused]]										\
static void BF_PASTE(CompileTimeTest_line_, __LINE__)


namespace BF {


// === AssertIsLValue(), AssertIsRValue() ==============================================================================

template <class Type>
void AssertIsLValue(Type&&)	{ static_assert( std::is_lvalue_reference_v<Type>, "This expression is not an lvalue."); }

template <class Type>
void AssertIsRValue(Type&&)	{ static_assert(!std::is_lvalue_reference_v<Type>, "This expression is not an rvalue."); }


// === LVal() ==========================================================================================================

template <class Type>
constexpr Type& LVal(Type&& value) noexcept
{
	return value;
}


// === PoisonedAddrOpClass =============================================================================================

struct PoisonedAddrOpClass {
	PoisonedAddrOpClass*                operator&()                = delete;
	const PoisonedAddrOpClass*          operator&() const          = delete;
	volatile PoisonedAddrOpClass*       operator&() volatile       = delete;
	const volatile PoisonedAddrOpClass* operator&() const volatile = delete;
};


// === MemsetValue =====================================================================================================

template <class Type>
class MemsetValue : ImmobileClass {
public:
	static_assert(IsDecayed<Type>, "'Type' should be decayed.");

	template <class... Pars>
	MemsetValue(Pars&&... pars)			{ ::new (&mValue) Type(BF_FWD(pars)...); }

	~MemsetValue()						{ mValue.~Type(); SecureMemset(&mValue, 0xBB, sizeof(mValue)); }

	Type&					GetL()		{ return mValue; }
	const Type&				GetCL()		{ return mValue; }
	volatile Type&			GetVL()		{ return mValue; }
	const volatile Type&	GetCVL()	{ return mValue; }

	Type&&					GetR()		{ return std::move(mValue); }
	const Type&&			GetCR()		{ return std::move(mValue); }
	volatile Type&&			GetVR()		{ return std::move(mValue); }
	const volatile Type&&	GetCVR()	{ return std::move(mValue); }

private:
	union { Type mValue; };
};


}	// namespace BF
