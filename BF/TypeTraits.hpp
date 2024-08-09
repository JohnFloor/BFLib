// Custom type traits.


#pragma once
#include <type_traits>
#include "BF/Definitions.hpp"


namespace BF {


// === PrintType =======================================================================================================

template <class... Ts>
struct PrintTypeT : std::true_type {
	static_assert(false, "See the type in the error message.");
};

template <class... Ts>
concept PrintType = PrintTypeT<Ts...>::value;


// === DontDeduce ======================================================================================================

template <class T>
using			DontDeduce = std::type_identity_t<T>;


// === Decayed =========================================================================================================

template <class T>
struct			IsDecayedT : std::is_same<T, std::decay_t<T>> {};

template <class T>
constexpr bool	IsDecayed = IsDecayedT<T>::value;

template <class T>
concept			Decayed = IsDecayedT<T>::value;


// === RelatedTo =======================================================================================================

template <class T1, class T2>
struct			AreRelatedT : std::disjunction<std::is_base_of<T1, T2>, std::is_base_of<T2, T1>> {};

template <class T1, class T2>
constexpr bool	AreRelated = AreRelatedT<T1, T2>::value;

template <class T1, class T2>
concept			RelatedTo = AreRelatedT<T1, T2>::value;


// === Referenceable ===================================================================================================

template <class T>
struct			IsReferenceableT : std::is_reference<std::add_lvalue_reference_t<T>> {};

template <class T>
constexpr bool	IsReferenceable = IsReferenceableT<T>::value;

template <class T>
concept			Referenceable = IsReferenceableT<T>::value;


// === Abominable ======================================================================================================

template <class T>
struct			IsAbominableT : std::conjunction<std::is_function<T>, std::negation<IsReferenceableT<T>>> {};

template <class T>
constexpr bool	IsAbominable = IsAbominableT<T>::value;

template <class T>
concept			Abominable = IsAbominableT<T>::value;


// === AreConstRelated =================================================================================================

template <class T1, class T2>	struct AreConstRelatedT                                       : std::is_same<T1, T2> {};

template <class T1, class T2>	struct AreConstRelatedT<T1,                const T2>          : AreConstRelatedT<T1, T2> {};
template <class T1, class T2>	struct AreConstRelatedT<T1,                volatile T2>       : std::false_type {};
template <class T1, class T2>	struct AreConstRelatedT<T1,                const volatile T2> : std::false_type {};

template <class T1, class T2>	struct AreConstRelatedT<const T1,          T2>                : AreConstRelatedT<T1, T2> {};
template <class T1, class T2>	struct AreConstRelatedT<const T1,          const T2>          : AreConstRelatedT<T1, T2> {};
template <class T1, class T2>	struct AreConstRelatedT<const T1,          volatile T2>       : std::false_type {};
template <class T1, class T2>	struct AreConstRelatedT<const T1,          const volatile T2> : std::false_type {};

template <class T1, class T2>	struct AreConstRelatedT<volatile T1,       T2>                : std::false_type {};
template <class T1, class T2>	struct AreConstRelatedT<volatile T1,       const T2>          : std::false_type {};
template <class T1, class T2>	struct AreConstRelatedT<volatile T1,       volatile T2>       : AreConstRelatedT<T1, T2> {};
template <class T1, class T2>	struct AreConstRelatedT<volatile T1,       const volatile T2> : AreConstRelatedT<T1, T2> {};

template <class T1, class T2>	struct AreConstRelatedT<const volatile T1, T2>                : std::false_type {};
template <class T1, class T2>	struct AreConstRelatedT<const volatile T1, const T2>          : std::false_type {};
template <class T1, class T2>	struct AreConstRelatedT<const volatile T1, volatile T2>       : AreConstRelatedT<T1, T2> {};
template <class T1, class T2>	struct AreConstRelatedT<const volatile T1, const volatile T2> : AreConstRelatedT<T1, T2> {};

template <class T1, class T2>	struct AreConstRelatedT<T1*,               T2*>               : AreConstRelatedT<T1, T2> {};
template <class T1, class T2>	struct AreConstRelatedT<T1&,               T2&>               : AreConstRelatedT<T1, T2> {};
template <class T1, class T2>	struct AreConstRelatedT<T1&&,              T2&&>              : AreConstRelatedT<T1, T2> {};

template <class Type1, class Type2>
constexpr bool AreConstRelated = AreConstRelatedT<Type1, Type2>::value;


// === NotSelf =========================================================================================================
// Used to prevent forwarding reference ctors./assignments from acting as copy/move ctor./assignment.

template <class Par, class Self>
concept NotSelf = !std::is_base_of_v<Self, std::remove_cvref_t<Par>>;


// === AddConstBeforeRef, AddVolatileBeforeRef, AddConstVolatileBeforeRef ==============================================

template <class Type>	struct AddConstBeforeRefT				  : std::type_identity<const Type> {};
template <class Type>	struct AddConstBeforeRefT<Type&>		  : std::type_identity<const Type&> {};
template <class Type>	struct AddConstBeforeRefT<Type&&>		  : std::type_identity<const Type&&> {};

template <class Type>	struct AddVolatileBeforeRefT			  : std::type_identity<volatile Type> {};
template <class Type>	struct AddVolatileBeforeRefT<Type&>		  : std::type_identity<volatile Type&> {};
template <class Type>	struct AddVolatileBeforeRefT<Type&&>	  : std::type_identity<volatile Type&&> {};

template <class Type>	struct AddConstVolatileBeforeRefT		  : std::type_identity<const volatile Type> {};
template <class Type>	struct AddConstVolatileBeforeRefT<Type&>  : std::type_identity<const volatile Type&> {};
template <class Type>	struct AddConstVolatileBeforeRefT<Type&&> : std::type_identity<const volatile Type&&> {};

template <class Type>	using AddConstBeforeRef			= AddConstBeforeRefT<Type>::type;
template <class Type>	using AddVolatileBeforeRef		= AddVolatileBeforeRefT<Type>::type;
template <class Type>	using AddConstVolatileBeforeRef	= AddConstVolatileBeforeRefT<Type>::type;


// === RemoveConstBeforeRef, RemoveVolatileBeforeRef, RemoveConstVolatileBeforeRef =====================================

template <class Type>	struct RemoveConstBeforeRefT					 : std::type_identity<Type> {};
template <class Type>	struct RemoveConstBeforeRefT<const Type>		 : std::type_identity<Type> {};
template <class Type>	struct RemoveConstBeforeRefT<const Type&>		 : std::type_identity<Type&> {};
template <class Type>	struct RemoveConstBeforeRefT<const Type&&>		 : std::type_identity<Type&&> {};

template <class Type>	struct RemoveVolatileBeforeRefT					 : std::type_identity<Type> {};
template <class Type>	struct RemoveVolatileBeforeRefT<volatile Type>	 : std::type_identity<Type> {};
template <class Type>	struct RemoveVolatileBeforeRefT<volatile Type&>	 : std::type_identity<Type&> {};
template <class Type>	struct RemoveVolatileBeforeRefT<volatile Type&&> : std::type_identity<Type&&> {};

template <class Type>	struct RemoveConstVolatileBeforeRefT : RemoveVolatileBeforeRefT<typename RemoveConstBeforeRefT<Type>::type> {};

template <class Type>	using RemoveConstBeforeRef		   = RemoveConstBeforeRefT<Type>::type;
template <class Type>	using RemoveVolatileBeforeRef	   = RemoveVolatileBeforeRefT<Type>::type;
template <class Type>	using RemoveConstVolatileBeforeRef = RemoveConstVolatileBeforeRefT<Type>::type;


// === std26::copy_cvref_t =============================================================================================
// TODO-C++26: Replace with std::copy_cvref.
// Status: https://wg21.link/P1450/status.
// Specification: https://wg21.link/P1450r0#page=11, in the last table. The definition was changed in r1 (which is probably a bug).

namespace std26 {
	template <class From, class To>   struct copy_cvref								: std::type_identity<To> {};
	template <class From, class To>   struct copy_cvref<const From, To>				: std::type_identity<AddConstBeforeRef<To>> {};
	template <class From, class To>   struct copy_cvref<volatile From, To>			: std::type_identity<AddVolatileBeforeRef<To>> {};
	template <class From, class To>   struct copy_cvref<const volatile From, To>	: std::type_identity<AddConstVolatileBeforeRef<To>> {};
	template <class From, class To>   struct copy_cvref<From&, To>					: std::type_identity<To&> {};
	template <class From, class To>   struct copy_cvref<const From&, To>			: std::type_identity<AddConstBeforeRef<To>&> {};
	template <class From, class To>   struct copy_cvref<volatile From&, To>			: std::type_identity<AddVolatileBeforeRef<To>&> {};
	template <class From, class To>   struct copy_cvref<const volatile From&, To>	: std::type_identity<AddConstVolatileBeforeRef<To>&> {};
	template <class From, class To>   struct copy_cvref<From&&, To>					: std::type_identity<To&&> {};
	template <class From, class To>   struct copy_cvref<const From&&, To>			: std::type_identity<AddConstBeforeRef<To>&&> {};
	template <class From, class To>   struct copy_cvref<volatile From&&, To>		: std::type_identity<AddVolatileBeforeRef<To>&&> {};
	template <class From, class To>   struct copy_cvref<const volatile From&&, To>	: std::type_identity<AddConstVolatileBeforeRef<To>&&> {};

	template <class From, class To>   using copy_cvref_t = copy_cvref<From, To>::type;
}


}	// namespace BF
