// Hashing ranges.


#pragma once
#include <random>
#include <ranges>
#include "BF/Hash.hpp"
#include "BF/RawMemory.hpp"


namespace BF {


// === Implementation details ==========================================================================================

namespace ImpHash {


template <std::ranges::random_access_range Range>
constexpr std::size_t GetRangeHash(const Range& range)
{
	const std::random_access_iterator auto begin = std::ranges::begin(range);
	const UInt64                           size  = std::ranges::size(range);

	ImpHash::HashCombinator hc;
	hc.Add(size);

	if (size <= 16) {
		for (const auto& value : range)
			hc.Add(value);
	} else {
		hc.Add(begin[0],        begin[1],        begin[2],        begin[3]);
		hc.Add(begin[size - 4], begin[size - 3], begin[size - 2], begin[size - 1]);

		std::ranlux48_base rng(hc.Get());
		const UInt64 baseIntervalSize = (size - 8) / 8;
		const UInt8  remainder        = (size - 8) % 8;
		UInt64       start            = 4;

		for (UInt8 i = 0; i < 8; i++) {
			const UInt64 intervalSize = baseIntervalSize + (i < remainder);
			hc.Add(begin[start + rng() % intervalSize]);
			start += intervalSize;
		}
	}

	return hc.Get();
}


}	// namespace ImpHash


// === class HashRange =================================================================================================

template <class Range>
class HashRange final {
private:
	static_assert(std::ranges::random_access_range<std::remove_cvref_t<Range>>, "'Range' must be a random access range.");
	static_assert(IsDecayed<Range> || std::is_array_v<Range>, "'Range' must be a decayed type or a C array.");
	using RangeElement = std::remove_reference_t<std::ranges::range_reference_t<Range>>;
	static_assert(IsDecayed<std::remove_const_t<RangeElement>>, "'Range' element type must be a (possibly const) decayed type.");

public:
	constexpr explicit HashRange(const Range& range) : mRange(range) {}

private:
	template <class Type>
	friend struct std::hash;

	const Range& mRange;
};


// === class HashRawMemory =============================================================================================

template <std::size_t Size>
class HashRawMemory final {
public:
	template <class Type>
	explicit HashRawMemory(const Type& value) :
		mArray(BF::AsByteArray(value))
	{
		static_assert(sizeof(Type) == Size, "'Type' has bad size.");
	}

private:
	template <class Type>
	friend struct std::hash;

	const std::byte (&mArray)[Size];
};


template <class Type>
HashRawMemory(const Type&) -> HashRawMemory<sizeof(Type)>;


}	// namespace BF


// === std::hash specializations =======================================================================================

template <class Range>
struct std::hash<BF::HashRange<Range>> {
	[[nodiscard]]
	constexpr static std::size_t operator()(BF::HashRange<Range> value) {
		return BF::ImpHash::GetRangeHash(value.mRange);
	}
};


template <std::size_t Size>
struct std::hash<BF::HashRawMemory<Size>> {
	[[nodiscard]]
	static std::size_t operator()(BF::HashRawMemory<Size> value) {
		return BF::ImpHash::GetRangeHash(value.mArray);
	}
};
