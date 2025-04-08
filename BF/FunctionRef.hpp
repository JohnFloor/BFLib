// BF::FunctionRef, a type-erased function view.


#pragma once
#include <cassert>
#include <cstdlib>
#include <memory>
#include "BF/RawMemory.hpp"
#include "BF/TypeTraits.hpp"


namespace BF {


// === Implementation details ==========================================================================================

namespace ImpFunctionRef {

	// BadFunctionRefWasCalled

template <class Ret, class... Pars>
BF_NOINLINE Ret BadFunctionRefWasCalled(GenPtr, Pars...) noexcept
{
	std::abort();
}

	// MemberPointer

template <class Type>
concept MemberPointer = std::is_member_pointer_v<Type>;

	// IsNoexceptFunction

template <class F>
struct IsNoexceptFunctionT {
	static_assert(false, "ILE: 'F' should be a cv/ref-unqualified function type.");
};

template <class Ret, class... Pars>
struct IsNoexceptFunctionT<Ret (Pars...)> : std::false_type {};

template <class Ret, class... Pars>
struct IsNoexceptFunctionT<Ret (Pars...) noexcept> : std::true_type {};

template <class F>
concept IsNoexceptFunction = IsNoexceptFunctionT<F>::value;

	// HasAnyFunctionCallOperator

template <class Type, class Ret, class... Pars>
struct HasAnyFunctionCallOperatorT {
	using UQ = std::remove_cvref_t<Type>;

	static_assert(std::is_class_v<UQ>, "ILE: 'Type' should be a (possibly cv/ref-qualified) class type.");

	static constexpr bool value =
		requires (Ret (UQ::*fp)(Pars...))					{ fp = &UQ::operator(); } ||
		requires (Ret (UQ::*fp)(Pars...) const)				{ fp = &UQ::operator(); } ||
		requires (Ret (UQ::*fp)(Pars...) volatile)			{ fp = &UQ::operator(); } ||
		requires (Ret (UQ::*fp)(Pars...) const volatile)	{ fp = &UQ::operator(); } ||
		requires (Ret (UQ::*fp)(Pars...) &)					{ fp = &UQ::operator(); } ||
		requires (Ret (UQ::*fp)(Pars...) const&)			{ fp = &UQ::operator(); } ||
		requires (Ret (UQ::*fp)(Pars...) volatile&)			{ fp = &UQ::operator(); } ||
		requires (Ret (UQ::*fp)(Pars...) const volatile&)	{ fp = &UQ::operator(); } ||
		requires (Ret (UQ::*fp)(Pars...) &&)				{ fp = &UQ::operator(); } ||
		requires (Ret (UQ::*fp)(Pars...) const&&)			{ fp = &UQ::operator(); } ||
		requires (Ret (UQ::*fp)(Pars...) volatile&&)		{ fp = &UQ::operator(); } ||
		requires (Ret (UQ::*fp)(Pars...) const volatile&&)	{ fp = &UQ::operator(); };
};

template <class Type, class Ret, class... Pars>
concept HasAnyFunctionCallOperator = HasAnyFunctionCallOperatorT<Type, Ret, Pars...>::value;

	// HasCallableFunctionCallOperator

struct CallTester {
	void X();
	void C()  const;
	void V()  volatile;
	void CV() const volatile;

	void R()   &;
	void CR()  const&;
	void VR()  volatile&;
	void CVR() const volatile&;

	void RR()   &&;
	void CRR()  const&&;
	void VRR()  volatile&&;
	void CVRR() const volatile&&;
};

template <class Type>
std26::copy_cvref_t<Type, CallTester>	AsCallTester();

template <class Type, auto CallTesterMethod>
concept IsCallable = requires { (AsCallTester<Type>().*CallTesterMethod)(); };

template <class Type, class Ret, class... Pars>
struct HasCallableFunctionCallOperatorT {
	using UQ = std::remove_cvref_t<Type>;

	static_assert(std::is_class_v<UQ>, "ILE: 'Type' should be a (possibly cv/ref-qualified) class type.");

	static constexpr bool value =
		IsCallable<Type, &CallTester::X>    && requires (Ret (UQ::*fp)(Pars...))					{ fp = &UQ::operator(); } ||
		IsCallable<Type, &CallTester::C>    && requires (Ret (UQ::*fp)(Pars...) const)				{ fp = &UQ::operator(); } ||
		IsCallable<Type, &CallTester::V>    && requires (Ret (UQ::*fp)(Pars...) volatile)			{ fp = &UQ::operator(); } ||
		IsCallable<Type, &CallTester::CV>   && requires (Ret (UQ::*fp)(Pars...) const volatile)		{ fp = &UQ::operator(); } ||
		IsCallable<Type, &CallTester::R>    && requires (Ret (UQ::*fp)(Pars...) &)					{ fp = &UQ::operator(); } ||
		IsCallable<Type, &CallTester::CR>   && requires (Ret (UQ::*fp)(Pars...) const&)				{ fp = &UQ::operator(); } ||
		IsCallable<Type, &CallTester::VR>   && requires (Ret (UQ::*fp)(Pars...) volatile&)			{ fp = &UQ::operator(); } ||
		IsCallable<Type, &CallTester::CVR>  && requires (Ret (UQ::*fp)(Pars...) const volatile&)	{ fp = &UQ::operator(); } ||
		IsCallable<Type, &CallTester::RR>   && requires (Ret (UQ::*fp)(Pars...) &&)					{ fp = &UQ::operator(); } ||
		IsCallable<Type, &CallTester::CRR>  && requires (Ret (UQ::*fp)(Pars...) const&&)			{ fp = &UQ::operator(); } ||
		IsCallable<Type, &CallTester::VRR>  && requires (Ret (UQ::*fp)(Pars...) volatile&&)			{ fp = &UQ::operator(); } ||
		IsCallable<Type, &CallTester::CVRR> && requires (Ret (UQ::*fp)(Pars...) const volatile&&)	{ fp = &UQ::operator(); };
};

template <class Type, class Ret, class... Pars>
concept HasCallableFunctionCallOperator = HasCallableFunctionCallOperatorT<Type, Ret, Pars...>::value;

	// AcceptableFunctor

template <class Functor, class Self, class Ret, class... Pars>
concept AcceptableFunctor =	NotSelf<Functor, Self> &&
							std::is_class_v<std::remove_cvref_t<Functor>> &&
							HasAnyFunctionCallOperator<Functor, Ret, Pars...>;

	// ReplaceTemplateArgument

template <class ClassTemplateInstantiation, class NewArg>
struct ReplaceTemplateArgumentT {
	static_assert(false, "ILE: 'ClassTemplateInstantiation' should be a class template instantiation with one type parameter.");
};

template <template <class> class ClassTemplate, class CurrentArg,  class NewArg>
struct ReplaceTemplateArgumentT<ClassTemplate<CurrentArg>, NewArg> : std::type_identity<ClassTemplate<NewArg>> {};

template <class ClassTemplateInstantiation, class NewArg>
using ReplaceTemplateArgument = ReplaceTemplateArgumentT<ClassTemplateInstantiation, NewArg>::type;

	// ConstCastResult

template <class Self, class ToSignature>
using ConstCastResult = std26::copy_cvref_t<Self, ReplaceTemplateArgument<std::remove_cvref_t<Self>, ToSignature>>;

	// SignatureFromType

struct DeductionGuideError : std::type_identity<void (DeductionGuideError)> {};

template <class>								 struct SignatureFromMethodT;
template <class Ret, class Type, class... Pars>	 struct SignatureFromMethodT<Ret (Type::*)(Pars...)>							{ using type = Ret (Pars...);       };
template <class Ret, class Type, class... Pars>	 struct SignatureFromMethodT<Ret (Type::*)(Pars...) const>						{ using type = Ret (Pars...) const; };
template <class Ret, class Type, class... Pars>	 struct SignatureFromMethodT<Ret (Type::*)(Pars...) volatile>					{ using type = Ret (Pars...);       };
template <class Ret, class Type, class... Pars>	 struct SignatureFromMethodT<Ret (Type::*)(Pars...) const volatile>				{ using type = Ret (Pars...) const; };
template <class Ret, class Type, class... Pars>	 struct SignatureFromMethodT<Ret (Type::*)(Pars...) &>							{ using type = Ret (Pars...);       };
template <class Ret, class Type, class... Pars>	 struct SignatureFromMethodT<Ret (Type::*)(Pars...) const&>						{ using type = Ret (Pars...) const; };
template <class Ret, class Type, class... Pars>	 struct SignatureFromMethodT<Ret (Type::*)(Pars...) volatile&>					{ using type = Ret (Pars...);       };
template <class Ret, class Type, class... Pars>	 struct SignatureFromMethodT<Ret (Type::*)(Pars...) const volatile&>			{ using type = Ret (Pars...) const; };
template <class Ret, class Type, class... Pars>	 struct SignatureFromMethodT<Ret (Type::*)(Pars...) &&>							{ using type = Ret (Pars...);       };
template <class Ret, class Type, class... Pars>	 struct SignatureFromMethodT<Ret (Type::*)(Pars...) const&&>					{ using type = Ret (Pars...) const; };
template <class Ret, class Type, class... Pars>	 struct SignatureFromMethodT<Ret (Type::*)(Pars...) volatile&&>					{ using type = Ret (Pars...);       };
template <class Ret, class Type, class... Pars>	 struct SignatureFromMethodT<Ret (Type::*)(Pars...) const volatile&&>			{ using type = Ret (Pars...) const; };
template <class Ret, class Type, class... Pars>	 struct SignatureFromMethodT<Ret (Type::*)(Pars...) noexcept>					{ using type = Ret (Pars...)       noexcept; };
template <class Ret, class Type, class... Pars>	 struct SignatureFromMethodT<Ret (Type::*)(Pars...) const noexcept>				{ using type = Ret (Pars...) const noexcept; };
template <class Ret, class Type, class... Pars>	 struct SignatureFromMethodT<Ret (Type::*)(Pars...) volatile noexcept>			{ using type = Ret (Pars...)       noexcept; };
template <class Ret, class Type, class... Pars>	 struct SignatureFromMethodT<Ret (Type::*)(Pars...) const volatile noexcept>	{ using type = Ret (Pars...) const noexcept; };
template <class Ret, class Type, class... Pars>	 struct SignatureFromMethodT<Ret (Type::*)(Pars...) & noexcept>					{ using type = Ret (Pars...)       noexcept; };
template <class Ret, class Type, class... Pars>	 struct SignatureFromMethodT<Ret (Type::*)(Pars...) const& noexcept>			{ using type = Ret (Pars...) const noexcept; };
template <class Ret, class Type, class... Pars>	 struct SignatureFromMethodT<Ret (Type::*)(Pars...) volatile& noexcept>			{ using type = Ret (Pars...)       noexcept; };
template <class Ret, class Type, class... Pars>	 struct SignatureFromMethodT<Ret (Type::*)(Pars...) const volatile& noexcept>	{ using type = Ret (Pars...) const noexcept; };
template <class Ret, class Type, class... Pars>	 struct SignatureFromMethodT<Ret (Type::*)(Pars...) && noexcept>				{ using type = Ret (Pars...)       noexcept; };
template <class Ret, class Type, class... Pars>	 struct SignatureFromMethodT<Ret (Type::*)(Pars...) const&& noexcept>			{ using type = Ret (Pars...) const noexcept; };
template <class Ret, class Type, class... Pars>	 struct SignatureFromMethodT<Ret (Type::*)(Pars...) volatile&& noexcept>		{ using type = Ret (Pars...)       noexcept; };
template <class Ret, class Type, class... Pars>	 struct SignatureFromMethodT<Ret (Type::*)(Pars...) const volatile&& noexcept>	{ using type = Ret (Pars...) const noexcept; };

template <class Function>
struct SignatureFromFunctionT : std::type_identity<Function> {};

template <class Type>
struct SignatureFromClassT : DeductionGuideError {
	static_assert(false, "'Type' has zero or more than one operator(). Could not deduce 'Signature' for 'BF::FunctionRef'.");
};

template <class Type>
requires requires { &Type::operator(); }
struct SignatureFromClassT<Type> : SignatureFromMethodT<decltype(&Type::operator())> {};

template <class Type>
struct SignatureFromNotClassT : DeductionGuideError {
	static_assert(false, "'Type' is not callable. Could not deduce 'Signature' for 'BF::FunctionRef'.");
};

template <class Type>
requires std::is_function_v<Type>
struct SignatureFromNotClassT<Type*> : SignatureFromFunctionT<Type> {};

template <class Type>
using SignatureFromType = std::conditional_t<std::is_class_v<Type>, SignatureFromClassT<Type>, SignatureFromNotClassT<Type>>::type;

	// Base

template <bool IsConst, bool IsNoexcept, class Ret, class... Pars>
class Base {
private:
	template <bool, bool, class, class...>
	friend class Base;

	template <class Pointee>
	static Ret FunctionForwarder(GenPtr genPtr, Pars... pars) noexcept(IsNoexcept) {
		if constexpr (IsNoexcept) {
			static_assert((std::is_nothrow_move_constructible_v<Pars> && ...), "A parameter's move ctor. or dtor. is not 'noexcept'.");
			static_assert(IsNoexceptFunction<Pointee>, "This BF::FunctionRef can only point to 'noexcept' functions.");
		}

		return genPtr.AsRef<Pointee>()(BF_FWD(pars)...);
	}

	template <class Pointee>
	static Ret FunctorForwarder(GenPtr genPtr, Pars... pars) noexcept(IsNoexcept) {
		using ConstPointee = std::conditional_t<IsConst, AddConstBeforeRef<Pointee>, Pointee>;

		if constexpr (!HasCallableFunctionCallOperator<ConstPointee, Ret, Pars...>) {
			using NonCVPointee = RemoveConstVolatileBeforeRef<ConstPointee>;

			if constexpr (HasCallableFunctionCallOperator<NonCVPointee, Ret, Pars...>)
				static_assert(false, "Cannot call operator()(Pars...); the pointee would lose some cv-qualifiers.");
			else if constexpr (HasCallableFunctionCallOperator<ConstPointee&, Ret, Pars...>)
				static_assert(false, "Cannot call operator()(Pars...); the pointee would have to be an lvalue.");
			else if constexpr (HasCallableFunctionCallOperator<std::remove_reference_t<ConstPointee>, Ret, Pars...>)
				static_assert(false, "Cannot call operator()(Pars...); the pointee would have to be an rvalue.");
			else if constexpr (HasCallableFunctionCallOperator<NonCVPointee&, Ret, Pars...>)
				static_assert(false, "Cannot call operator()(Pars...); the pointee would lose some cv-qualifiers, and would have to be an lvalue.");
			else if constexpr (HasCallableFunctionCallOperator<std::remove_reference_t<NonCVPointee>, Ret, Pars...>)
				static_assert(false, "Cannot call operator()(Pars...); the pointee would lose some cv-qualifiers, and would have to be an rvalue.");
			else
				static_assert(false, "ILE: Unhandled case while determining why operator()(Pars...) cannot be called.");
		}

		if constexpr (IsNoexcept) {
			constexpr bool ParametersOK = (std::is_nothrow_move_constructible_v<Pars> && ...);
			constexpr bool CallOK = noexcept(genPtr.AsRef<ConstPointee>()(BF_FWD(pars)...));		// subsumes ParametersOK
			static_assert(ParametersOK, "A parameter's move ctor. or dtor. is not 'noexcept'.");
			static_assert(!ParametersOK || CallOK, "The operator() to be called is not marked 'noexcept'.");
		}

		return genPtr.AsRef<ConstPointee>()(BF_FWD(pars)...);
	}

	template <bool SourceIsConst, bool SourceIsNoexcept>
	void CopyFriend(const Base<SourceIsConst, SourceIsNoexcept, Ret, Pars...>& source) {
		static_assert(SourceIsConst    >= IsConst,    "Signature of source BF::FunctionRef should be also const.");
		static_assert(SourceIsNoexcept >= IsNoexcept, "Signature of source BF::FunctionRef should be also noexcept.");

		mGenPtr    = source.mGenPtr;
		mForwarder = source.mForwarder;
	}

	template <bool SourceIsConst, bool SourceIsNoexcept>
	void CopyFriend(const volatile Base<SourceIsConst, SourceIsNoexcept, Ret, Pars...>& source) {
		static_assert(false, "Copy-constructing/assigning from a volatile source would lose volatile qualifier.");
	}

	template <class Function>
	void SetFromFunction(Function* functionPtr) {
		assert(functionPtr != nullptr);

		mGenPtr    = functionPtr;
		mForwarder = &FunctionForwarder<Function>;
	}

	template <class Functor>
	void SetFromFunctor(Functor&& functor) {
		assert(!IsNullReference(functor));

		if constexpr (requires { CopyFriend(functor); }) {		// if std::decay_t<Functor> is a Base<*, *, Ret, Pars...>
			CopyFriend(functor);
		} else {
			mGenPtr    = std::addressof(functor);
			mForwarder = &FunctorForwarder<Functor>;
		}
	}

	template <bool ToIsConst, bool ToIsNoexcept, class ToRet, class... ToPars>
	static void CheckToSignatureForConstCast(Base<ToIsConst, ToIsNoexcept, ToRet, ToPars...>*) {
		if constexpr (IsConst)
			static_assert(ToIsConst, "'Signature' of BF::FunctionRef is const, therefore 'ToSignature' of ConstCast() should also be const.");
		else
			static_assert(!ToIsConst, "'Signature' of BF::FunctionRef is not const, therefore 'ToSignature' of ConstCast() should also be not const.");

		if constexpr (IsNoexcept)
			static_assert(ToIsNoexcept, "'Signature' of BF::FunctionRef is noexcept, therefore 'ToSignature' of ConstCast() should also be noexcept.");
		else
			static_assert(!ToIsNoexcept, "'Signature' of BF::FunctionRef is not noexcept, therefore 'ToSignature' of ConstCast() should also be not noexcept.");

		static_assert(sizeof...(ToPars) == sizeof...(Pars), "'ToSignature' of ConstCast() should contain the same number of parameters as 'Signature' of BF::FunctionRef.");
		static_assert(AreConstRelated<Ret, ToRet>, "ConstCast() can only change the constness of the return type.");
		static_assert((AreConstRelated<Pars, ToPars> && ...), "ConstCast() can only change the constness of the parameter types.");
	}

public:
	Base() : Base(Bad) {}

	Base(BadSelector) {
		mGenPtr    = Bad;
		mForwarder = &BadFunctionRefWasCalled<Ret, Pars...>;
	}

	[[gsl::suppress(type.6)]]						// member variables are intentionally uninitialized
	Base(UninitializedSelector) {}

	Base(std::nullptr_t) = delete;					// this reference is not nullable; '== nullptr' also cannot be checked

	Base(MemberPointer auto) = delete;				// member pointers are not supported

	Base(Ret (*functionPtr)(Pars...)) {
		SetFromFunction(functionPtr);
	}

	Base(Ret (*functionPtr)(Pars...) noexcept) {
		SetFromFunction(functionPtr);
	}

	Base(AcceptableFunctor<Base, Ret, Pars...> auto&& functor) {
		SetFromFunctor(BF_FWD(functor));
	}

	Base& operator=(std::nullptr_t) = delete;		// this reference is not nullable; '== nullptr' also cannot be checked

	Base& operator=(MemberPointer auto) = delete;	// member pointers are not supported

	Base& operator=(Ret (*newFunctionPtr)(Pars...)) {
		SetFromFunction(newFunctionPtr);
		return *this;
	}

	Base& operator=(Ret (*newFunctionPtr)(Pars...) noexcept) {
		SetFromFunction(newFunctionPtr);
		return *this;
	}

	Base& operator=(AcceptableFunctor<Base, Ret, Pars...> auto&& newFunctor) {
		SetFromFunctor(BF_FWD(newFunctor));
		return *this;
	}

	template <class Self>
	RemoveConstBeforeRef<Self>&& ConstCast(this Self&& self) {
		static_assert(std::is_const_v<std::remove_reference_t<Self>>, "No need to cast away constness, '*this' is already not const.");
		static_assert(!IsConst, "No need to cast away constness, 'operator()' is const.");

		return const_cast<RemoveConstBeforeRef<Self>&&>(self);
	}

	template <class ToSignature, class Self>
	ConstCastResult<Self, ToSignature>&& ConstCast(this Self&& self) {
		ReplaceTemplateArgument<std::remove_cvref_t<Self>, ToSignature>* dummyFunctionRefWithToSignature = nullptr;
		CheckToSignatureForConstCast(dummyFunctionRefWithToSignature);

		return reinterpret_cast<ConstCastResult<Self, ToSignature>&&>(self);
	}

protected:
	GenPtr mGenPtr;
	Ret  (*mForwarder)(GenPtr, Pars...) noexcept(IsNoexcept);
};


}	// namespace ImpFunctionRef


// === class FunctionRef ===============================================================================================

template <class Signature>
class FunctionRef final {
	static_assert(false, "'Signature' must be a function type in the form 'Ret (Pars...) [const] [noexcept]'.");
};


template <class Ret, class... Pars>
class FunctionRef<Ret (Pars...)> final : public ImpFunctionRef::Base<false, false, Ret, Pars...> {
	using Base = ImpFunctionRef::Base<false, false, Ret, Pars...>;

public:
	using Base::Base;
	using Base::operator=;

	Ret operator()(Pars... pars) {
		return this->mForwarder(this->mGenPtr, BF_FWD(pars)...);
	}
};


template <class Ret, class... Pars>
class FunctionRef<Ret (Pars...) const> final : public ImpFunctionRef::Base<true, false, Ret, Pars...> {
	using Base = ImpFunctionRef::Base<true, false, Ret, Pars...>;

public:
	using Base::Base;
	using Base::operator=;

	Ret operator()(Pars... pars) const {
		return this->mForwarder(this->mGenPtr, BF_FWD(pars)...);
	}
};


template <class Ret, class... Pars>
class FunctionRef<Ret (Pars...) noexcept> final : public ImpFunctionRef::Base<false, true, Ret, Pars...> {
	using Base = ImpFunctionRef::Base<false, true, Ret, Pars...>;

public:
	using Base::Base;
	using Base::operator=;

	Ret operator()(Pars... pars) noexcept {
		return this->mForwarder(this->mGenPtr, BF_FWD(pars)...);
	}
};


template <class Ret, class... Pars>
class FunctionRef<Ret (Pars...) const noexcept> final : public ImpFunctionRef::Base<true, true, Ret, Pars...> {
	using Base = ImpFunctionRef::Base<true, true, Ret, Pars...>;

public:
	using Base::Base;
	using Base::operator=;

	Ret operator()(Pars... pars) const noexcept {
		return this->mForwarder(this->mGenPtr, BF_FWD(pars)...);
	}
};


template <>
class FunctionRef<void (ImpFunctionRef::DeductionGuideError)> final {
public:
	FunctionRef(auto&&) {}		// avoid irrelevant error messages by converting from any type
};


template <class Type>
FunctionRef(Type) -> FunctionRef<ImpFunctionRef::SignatureFromType<Type>>;


}	// namespace BF
