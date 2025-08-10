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


// === Integer =========================================================================================================

template <class T>
constexpr bool	IsInteger = std::is_integral_v<T> && !std::is_same_v<std::remove_cv_t<T>, bool>;

template <class T>
struct			IsIntegerT : std::bool_constant<IsInteger<T>> {};

template <class T>
concept			Integer = IsInteger<T>;


// === SInteger ========================================================================================================

template <class T>
constexpr bool	IsSInteger = IsInteger<T> && std::is_signed_v<T>;

template <class T>
struct			IsSIntegerT : std::bool_constant<IsSInteger<T>> {};

template <class T>
concept			SInteger = Integer<T> && std::is_signed_v<T>;


// === UInteger ========================================================================================================

template <class T>
constexpr bool	IsUInteger = IsInteger<T> && std::is_unsigned_v<T>;

template <class T>
struct			IsUIntegerT : std::bool_constant<IsUInteger<T>> {};

template <class T>
concept			UInteger = Integer<T> && std::is_unsigned_v<T>;


// === RelatedTo =======================================================================================================

template <class T1, class T2>
constexpr bool	AreRelated = std::is_base_of_v<T1, T2> || std::is_base_of_v<T2, T1>;

template <class T1, class T2>
struct			AreRelatedT : std::bool_constant<AreRelated<T1, T2>> {};

template <class T1, class T2>
concept			RelatedTo = AreRelated<T1, T2>;


// === Referenceable ===================================================================================================

template <class T>
constexpr bool	IsReferenceable = std::is_reference_v<std::add_lvalue_reference_t<T>>;

template <class T>
struct			IsReferenceableT : std::bool_constant<IsReferenceable<T>> {};

template <class T>
concept			Referenceable = IsReferenceable<T>;


// === Abominable ======================================================================================================

template <class T>
constexpr bool	IsAbominable = std::is_function_v<T> && !IsReferenceable<T>;

template <class T>
struct			IsAbominableT : std::bool_constant<IsAbominable<T>> {};

template <class T>
concept			Abominable = IsAbominable<T>;


// === Decayed =========================================================================================================

template <class T>
constexpr bool	IsDecayed = std::is_same_v<T, std::decay_t<T>>;

template <class T>
struct			IsDecayedT : std::bool_constant<IsDecayed<T>> {};

template <class T>
concept			Decayed = IsDecayed<T>;


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


// === std29::copy_cvref_t =============================================================================================
// TODO-C++29: Replace with std::copy_cvref, if it is defined the same way as here.
// Status: https://wg21.link/P1450/status.
// Specification: https://wg21.link/P1450r0#page=11, in the last table. The definition was changed in r1 (which is probably a bug).

namespace std29 {
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


// === std29::copy_cv_t ================================================================================================
// TODO-C++29: Replace with std::copy_cv.
// Status: https://wg21.link/P1450/status.
// Specification: https://wg21.link/P1450r3#page=8, in the first table.

namespace std29 {
	template <class From, class To>   struct copy_cv							: std::type_identity<To> {};
	template <class From, class To>   struct copy_cv<const From, To>			: std::type_identity<const To> {};
	template <class From, class To>   struct copy_cv<volatile From, To>			: std::type_identity<volatile To> {};
	template <class From, class To>   struct copy_cv<const volatile From, To>	: std::type_identity<const volatile To> {};

	template <class From, class To>   using copy_cv_t = copy_cv<From, To>::type;
}


}	// namespace BF
