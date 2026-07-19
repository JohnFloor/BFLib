// Compare utilities.


#pragma once
#include <compare>
#include <type_traits>


namespace BF {


// === StdOrdering =====================================================================================================

template <class Type>
constexpr bool	IsStdOrdering = std::is_same_v<Type, std::strong_ordering> ||
								std::is_same_v<Type, std::weak_ordering>   ||
								std::is_same_v<Type, std::partial_ordering>;

template <class Type>
struct			IsStdOrderingT : std::bool_constant<IsStdOrdering<Type>> {};

template <class Type>
concept			StdOrdering = IsStdOrdering<Type>;


}	// namespace BF
