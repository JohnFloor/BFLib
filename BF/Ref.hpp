// The class template Ref is a wrapper for reference classes, i.e. classes which have reference semantics.
// It adds const-correctness for the usage of such classes. Example in the test.


#pragma once
#include "BF/Compare.hpp"
#include "BF/Hash.hpp"
#include "BF/TypeTraits.hpp"


namespace BF {


// === class Ref =======================================================================================================

template <class Type>
class Ref {
	template <class>
	friend class Ref;

	static_assert(std::is_class_v<Type>, "'Type' should be a (possibly cv-qualified) class.");

private:
	using PType = std::remove_cv_t<Type>;

	template <class T>
	static void AcceptSingleRef(const volatile Ref<T>&);

	template <class... Pars>
	static constexpr bool IsSingleRef = requires (Pars&... pars) {
		AcceptSingleRef(pars...);
	};

	template <class SourceType>
	static decltype(auto) Verify(auto&& source) {
		static_assert(std::is_same_v<std::remove_cv_t<SourceType>, std::remove_cv_t<Type>>, "'SourceType' should be the same as 'Type', ignoring cv-qualifiers.");
		static_assert(std::is_const_v<SourceType>    <= std::is_const_v<Type>,    "Cannot construct/assign from 'BF::Ref<SourceType>'. Conversion would lose a const qualifier.");
		static_assert(std::is_volatile_v<SourceType> <= std::is_volatile_v<Type>, "Cannot construct/assign from 'BF::Ref<SourceType>'. Conversion would lose a volatile qualifier.");

		return BF_FWD(source);
	}

public:
	template <class... Pars>
	requires (!IsSingleRef<Pars...>)
	Ref(Pars&&... pars) :
		mRefValue(BF_FWD(pars)...)
	{
	}

	template <class SourceType>
	Ref(const Ref<SourceType>& source) :
		mRefValue(Verify<SourceType>(source.mRefValue))
	{
	}

	template <class SourceType>
	Ref(Ref<SourceType>&& source) :
		mRefValue(Verify<SourceType>(std::move(source.mRefValue)))
	{
	}

	template <class Par>
	requires (!IsSingleRef<Par>)
	Ref& operator=(Par&& par)
	{
		mRefValue = BF_FWD(par);
		return *this;
	}

	template <class SourceType>
	Ref& operator=(const Ref<SourceType>& source)
	{
		mRefValue = Verify<SourceType>(source.mRefValue);
		return *this;
	}

	template <class SourceType>
	Ref& operator=(Ref<SourceType>&& source)
	{
		mRefValue = Verify<SourceType>(std::move(source.mRefValue));
		return *this;
	}

	Type* operator->() const {
		return const_cast<Type*>(&mRefValue);
	}

	template <class ToType, class Self>
	decltype(auto) ConstCast(this Self&& self) {
		if constexpr (std::is_same_v<std::remove_cv_t<ToType>, std::remove_cv_t<Type>>)
			return reinterpret_cast<std29::copy_cvref_t<Self, Ref<ToType>>&&>(self);
		else
			static_assert(false, "'ToType' should be the same as 'Type', ignoring cv-qualifiers.");
	}

	bool operator==(const Ref& rightOp) const
	requires requires (const PType x) { { x == x } -> std::convertible_to<bool>; }
	{ return mRefValue == rightOp.mRefValue; }

	bool operator!=(const Ref& rightOp) const
	requires requires (const PType x) { { x != x } -> std::convertible_to<bool>; }
	{ return mRefValue != rightOp.mRefValue; }

	bool operator<(const Ref& rightOp) const
	requires requires (const PType x) { { x <  x } -> std::convertible_to<bool>; }
	{ return mRefValue < rightOp.mRefValue; }

	bool operator>(const Ref& rightOp) const
	requires requires (const PType x) { { x >  x } -> std::convertible_to<bool>; }
	{ return mRefValue > rightOp.mRefValue; }

	bool operator<=(const Ref& rightOp) const
	requires requires (const PType x) { { x <= x } -> std::convertible_to<bool>; }
	{ return mRefValue <= rightOp.mRefValue; }

	bool operator>=(const Ref& rightOp) const
	requires requires (const PType x) { { x >= x } -> std::convertible_to<bool>; }
	{ return mRefValue >= rightOp.mRefValue; }

	auto operator<=>(const Ref& rightOp) const
	requires requires (const PType x) { { x <=> x } -> BF::StdOrdering; }
	{ return mRefValue <=> rightOp.mRefValue; }

	BF::Hash BF_GetHash() const
	requires BF::StdHashable<PType>
	{ return { mRefValue }; }

private:
	std::remove_cv_t<Type>	mRefValue;
};


}	// namespace BF
