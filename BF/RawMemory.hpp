// Manipulating raw memory and void* pointers.


#pragma once
#include <memory>
#include "BF/TypeTraits.hpp"


namespace BF {


// === GenPtr ==========================================================================================================
// Can hold the address of an object or a function. Cannot hold a pointer to member.

union GenPtr {
public:
	template <class Type>						// 'Type' must be non-&.
	void operator=(Type* newPtr) noexcept {
		if constexpr (std::is_function_v<Type>)
			mFunPtr = reinterpret_cast<void(*)()>(newPtr);
		else
			mObjPtr = (void*)newPtr;
	}

	template <class Type>						// 'Type' must be non-&.
	Type* AsPtr() noexcept {
		if constexpr (std::is_function_v<Type>)
			return reinterpret_cast<Type*>(mFunPtr);
		else
			return static_cast<Type*>(mObjPtr);
	}

	template <class Type>						// 'Type' is non-&, && => AsRef() will be xvalue.
	Type&& AsRef() noexcept {					// 'Type' is &         => AsRef() will be lvalue.
		return static_cast<Type&&>(*AsPtr<std::remove_reference_t<Type>>());
	}

private:
	void (*mFunPtr)();
	void*  mObjPtr;
};


// === Bad values ======================================================================================================

constexpr UInt16  BadValue16  = 0xBAAD;
constexpr UInt32  BadValue32  = 0xBAADBAAD;
constexpr UInt64  BadValue64  = 0xBAADBAADBAADBAAD;
constexpr UIntPtr BadValuePtr = static_cast<UIntPtr>(BadValue64);


// === IsNullReference =================================================================================================

bool IsNullReference(auto&& value) noexcept
{
	return (char*)std::addressof(value) + 1 == (char*)nullptr + 1;
}


}	// namespace BF
