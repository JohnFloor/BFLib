// Manipulating raw memory and void* pointers.


#pragma once
#include <memory>
#include "BF/TypeTraits.hpp"


namespace BF {


// === GenPtr ==========================================================================================================
// Can hold the address of an object or a function. Cannot hold a pointer to member.

union GenPtr {
public:
	void operator=(BadSelector) noexcept {
		mObjPtr = (void*)BadValuePtr;
	}

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


// === AsByteArray() ===================================================================================================

template <class Type>
auto& AsByteArray(Type& value)
{
	constexpr bool ConstDecayedOrArray = IsDecayedOrArray<std::remove_const_t<Type>>;
	constexpr bool TriviallyCopyable   = ConstDecayedOrArray BF_IMPLIES std::is_trivially_copyable_v<Type>;
	constexpr bool UniqueReps          = TriviallyCopyable BF_IMPLIES std::has_unique_object_representations_v<Type>;

	static_assert(ConstDecayedOrArray, "'Type' must be decayed/const, or a bounded array of decayed/const types.");
	static_assert(TriviallyCopyable,   "'Type' must be trivially copyable.");
	static_assert(UniqueReps,          "A value of 'Type' can be represented by two distinct bit patterns. "
									   "E.g., it has paddings or a floating point member.");

	using CVByteArray = std29::copy_cv_t<Type, std::byte[sizeof(Type)]>;

	return reinterpret_cast<CVByteArray&>(value);
}


void AsByteArray(const auto&&) = delete;		// rvalues are not accepted


// === IsNullReference =================================================================================================

bool IsNullReference(auto&& value) noexcept
{
	return (char*)std::addressof(value) + 1 == (char*)nullptr + 1;
}


// === SecureMemset ====================================================================================================
// TODO-C23: Replace with memset_explicit().

inline void SecureMemset(void* startAddress, UChar value, size_t size)
{
	UChar* begin = static_cast<UChar*>(startAddress);
	UChar* end   = begin + size;

	for (volatile UChar* c = begin; c != end; c++)
		*c = value;
}


}	// namespace BF
