// Easier hashing with BF_GetHash() method and function.


#pragma once
#include <concepts>
#include "BF/TypeTraits.hpp"


namespace BF {


// === concept StdHashable =============================================================================================

template <class Type>
concept StdHashable = requires (const Type value) {
	{ std::hash<Type>()(value) } -> std::convertible_to<std::size_t>;
};


// === Implementation details ==========================================================================================

namespace ImpHash {


// https://en.wikipedia.org/wiki/Fowler-Noll-Vo_hash_function#FNV-1a_hash
class FNV1aCombinator {
public:
	constexpr FNV1aCombinator() {
		mHash = FNVOffsetBasis;
	}

	constexpr void Add(std::size_t newHash) {
		mHash ^= newHash;
		mHash *= FNVPrime;
	}

	constexpr std::size_t Get() const {
		return mHash;
	}

private:
	static_assert(sizeof(std::size_t) == 8, "This algorithm is for 64 bits.");

	constexpr static std::size_t FNVOffsetBasis = 14695981039346656037;
	constexpr static std::size_t FNVPrime       = 1099511628211;

	std::size_t mHash;
};


class HashCombinator : private FNV1aCombinator {
public:
	constexpr HashCombinator() = default;

	constexpr void Add(const auto&... values) {
		( ..., FNV1aCombinator::Add(GetHash(values)) );
	}

	constexpr std::size_t Get() const {
		return FNV1aCombinator::Get();
	}

	constexpr static std::size_t Combine(const auto&... values) {
		if constexpr (sizeof...(values) == 1) {
			return GetHash(values...);
		} else {
			HashCombinator hc;
			hc.Add(values...);
			return hc.Get();
		}
	}

private:
	template <class Type>		// If ordinary lookup finds a class member, there is no ADL; which is our intention.
	constexpr static std::size_t GetHash(const Type& value) {
		if constexpr (StdHashable<Type>) {
			return std::hash<Type>()(value);
		} else {
			static_assert(false, "'Type' is not hashable.");
			return 0;
		}
	}
};


}	// namespace ImpHash


// === class Hash ======================================================================================================

class Hash final {
public:
	constexpr Hash(const auto&... values) :
		mValue(ImpHash::HashCombinator::Combine(values...))
	{
		static_assert(sizeof...(values) >= 1, "BF::Hash: Provide at least one value to hash.");
	}

private:
	template <class Type>
	friend struct std::hash;

	constexpr operator std::size_t() const { return mValue; }		// for usage inside 'std::hash' specializations

	const std::size_t mValue;
};


// === Implementation details ==========================================================================================

namespace ImpHash {


// Reason for putting these deleted functions here:
//   - We want to detect 'BF_GetHash()' function for 'Type', iff its parameter is possibly cv/ref-qualified 'Type'.
//   - They stop ordinary lookup at this scope, thus 'BF_GetHash()' function will be found by ADL only. This prevents
//     #include order dependencies. Note: specifically, in this case, the user is unlikely to introduce such
//     dependencies, because to define an inline 'BF_GetHash()' method or function, the definition of BF::Hash is
//     necessary, therefore this header will most likely be #include-d in "Type.hpp". So, there will be only one order
//     possible: "BF/Hash.hpp", and then (the contents of) "Type.hpp".
void BF_GetHash(const volatile auto&)  = delete;
void BF_GetHash(const volatile auto&&) = delete;


template <class Type>
concept HasGetHashMethod   = requires (Type value) { value.BF_GetHash(); } ||
							 requires (Type value) { std::move(value).BF_GetHash(); };


template <class Type>
concept HasGetHashFunction = requires (Type value) { BF_GetHash(value); } ||
							 requires (Type value) { BF_GetHash(std::move(value)); };


template <class Type>
concept GetHashMethodIsConst   = requires (const Type value) { value.BF_GetHash(); };


template <class Type>
concept GetHashFunctionIsConst = requires (const Type value) { BF_GetHash(value); };


template <class Type>
concept GetHashMethodReturnsHash   = requires (const Type value) { { value.BF_GetHash() } -> std::same_as<Hash>; };


template <class Type>
concept GetHashFunctionReturnsHash = requires (const Type value) { { BF_GetHash(value) } -> std::same_as<Hash>; };


}	// namespace ImpHash


// === concept HasGetHash ==============================================================================================

template <class Type>
concept HasGetHash = ImpHash::HasGetHashMethod<Type> || ImpHash::HasGetHashFunction<Type>;


}	// namespace BF


// === std::hash specialization ========================================================================================

template <BF::HasGetHash Type>
struct std::hash<Type> {
	constexpr static bool HasMethod           = BF::ImpHash::HasGetHashMethod<Type>;
	constexpr static bool MethodIsConst       = BF::ImpHash::GetHashMethodIsConst<Type>;
	constexpr static bool MethodReturnsHash   = BF::ImpHash::GetHashMethodReturnsHash<Type>;

	constexpr static bool HasFunction         = BF::ImpHash::HasGetHashFunction<Type>;
	constexpr static bool FunctionIsConst     = BF::ImpHash::GetHashFunctionIsConst<Type>;
	constexpr static bool FunctionReturnsHash = BF::ImpHash::GetHashFunctionReturnsHash<Type>;

	static_assert(BF::IsDecayed<Type>, "'Type' must be decayed.");
	static_assert(!HasMethod || !HasFunction, "Ambiguous hashing: 'Type' has both a 'value.BF_GetHash()' method and a 'BF_GetHash(value)' function.");

	static_assert(HasMethod       BF_IMPLIES MethodIsConst,       "BF_GetHash() method should be const.");
	static_assert(MethodIsConst   BF_IMPLIES MethodReturnsHash,   "BF_GetHash() method should return BF::Hash.");
	static_assert(HasFunction     BF_IMPLIES FunctionIsConst,     "BF_GetHash() function's parameter should be const.");
	static_assert(FunctionIsConst BF_IMPLIES FunctionReturnsHash, "BF_GetHash() function should return BF::Hash.");

	[[nodiscard]]
	constexpr static std::size_t operator()(const Type& value) {
		if constexpr (HasMethod)
			return value.BF_GetHash();
		else
			return BF_GetHash(value);
	}
};
