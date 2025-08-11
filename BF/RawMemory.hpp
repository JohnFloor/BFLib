// Manipulating raw memory and void* pointers.


#pragma once
#include <memory>
#include "BF/Assert.hpp"
#include "BF/BasicMath.hpp"
#include "BF/ClassUtils.hpp"
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


// === RawRange ========================================================================================================

template <std::size_t Size, std::size_t Align>
requires (Size != 0 && IsPowerOf2(Align) && Size % Align == 0)
class RawRange final : ImmobileClass {
private:
	template <class I>
	constexpr static bool Divides = (Size % sizeof(I) == 0) && (Align % alignof(I) == 0);

	static auto ChooseInt() {
		if      constexpr (Divides<UInt64>) return UInt64();
		else if constexpr (Divides<UInt32>) return UInt32();
		else if constexpr (Divides<UInt16>) return UInt16();
		else                                return UInt8();
	}

	using Int = decltype(ChooseInt());

public:
	RawRange()       = delete;		// use 'BF::AsRawRange()' instead
	RawRange(auto&&) = delete;		// use 'BF::AsRawRange()' instead

	Int*                         begin()                             { return mArray;             }
	const Int*                   begin() const                       { return mArray;             }

	Int*                         end()                               { return mArray + GetSize(); }
	const Int*                   end() const                         { return mArray + GetSize(); }

	constexpr static std::size_t GetSize()                           { return sizeof(mArray) / sizeof(mArray[0]); }

	Int&                         operator[](std::size_t index)       { BF_ASSERT(index < GetSize()); return mArray[index]; }
	const Int&                   operator[](std::size_t index) const { BF_ASSERT(index < GetSize()); return mArray[index]; }

private:
	alignas(Align) Int mArray[Size / sizeof(Int)];
};


template <class... Types>
RawRange(Types&&...) -> RawRange<1, 1>;


template <class Type>
auto& AsRawRange(Type& value)
{
	constexpr bool ConstDecayedOrArray = IsDecayedOrArray<std::remove_const_t<Type>>;
	constexpr bool TriviallyCopyable   = ConstDecayedOrArray BF_IMPLIES std::is_trivially_copyable_v<Type>;
	constexpr bool UniqueReps          = TriviallyCopyable BF_IMPLIES std::has_unique_object_representations_v<Type>;

	static_assert(ConstDecayedOrArray, "'Type' must be decayed/const, or a bounded array of decayed/const types.");
	static_assert(TriviallyCopyable,   "'Type' must be trivially copyable.");
	static_assert(UniqueReps,          "A value of 'Type' can be represented by two distinct bit patterns. "
									   "E.g., it has paddings or a floating point member.");

	using CVRawRange = std29::copy_cv_t<Type, RawRange<sizeof(Type), alignof(Type)>>;

	return reinterpret_cast<CVRawRange&>(value);
}


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
