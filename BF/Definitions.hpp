// Basic language tools. #include'd everywhere without a second thought.


#pragma once
#include <cstddef>				// std::nullptr_t, std::size_t, std::byte
#include <cstdint>				// fixed width integer types
#include <initializer_list>		// std::initializer_list
#include <utility>				// std::move, std::forward


// === Fixed width integer types =======================================================================================

using Int8    = std::int8_t;
using Int16   = std::int16_t;
using Int32   = std::int32_t;
using Int64   = std::int64_t;
using IntPtr  = std::intptr_t;

using UInt8   = std::uint8_t;
using UInt16  = std::uint16_t;
using UInt32  = std::uint32_t;
using UInt64  = std::uint64_t;
using UIntPtr = std::uintptr_t;

constexpr Int8    MaxInt8    = INT8_MAX;
constexpr Int16   MaxInt16   = INT16_MAX;
constexpr Int32   MaxInt32   = INT32_MAX;
constexpr Int64   MaxInt64   = INT64_MAX;
constexpr IntPtr  MaxIntPtr  = INTPTR_MAX;

constexpr UInt8   MaxUInt8   = UINT8_MAX;
constexpr UInt16  MaxUInt16  = UINT16_MAX;
constexpr UInt32  MaxUInt32  = UINT32_MAX;
constexpr UInt64  MaxUInt64  = UINT64_MAX;
constexpr UIntPtr MaxUIntPtr = UINTPTR_MAX;


// === Other type aliases ==============================================================================================

using UChar = unsigned char;

constexpr UChar MaxUChar = UINT8_MAX;


// === BF_DEBUG, BF_RELEASE ============================================================================================

#ifdef NDEBUG
	#define BF_DEBUG	false
	#define BF_RELEASE	true
#else
	#define BF_DEBUG	true
	#define BF_RELEASE	false
#endif


// === BF_UNUSED_VAR ===================================================================================================

#define BF_UNUSED_VAR(...)				__VA_ARGS__


// === BF_NOINLINE =====================================================================================================

#define BF_NOINLINE						[[msvc::noinline]]


// === BF_IMPLIES ======================================================================================================

#define BF_IMPLIES						<=


// === BF_FWD ==========================================================================================================

#define BF_FWD(x)						std::forward<decltype(x)>(x)


// === BF_STRINGIFY ====================================================================================================

#define IMP_BF_STRINGIFY(token)			#token
#define BF_STRINGIFY(token)				IMP_BF_STRINGIFY(token)


// === BF_PASTE ========================================================================================================

#define IMP_BF_PASTE(token1, token2)	token1 ## token2
#define BF_PASTE(token1, token2)		IMP_BF_PASTE(token1, token2)


// === BF_COUNTED_NAME =================================================================================================

#define BF_COUNTED_NAME(prefix)			BF_PASTE(prefix, __COUNTER__)


// === BF_DUMMY_NAME, BF_DUMMY =========================================================================================

#define BF_DUMMY_NAME					BF_COUNTED_NAME(zzzz0z00z0zz0z_)
#define BF_DUMMY						BF_DUMMY_NAME [[maybe_unused]]


// === BF_SCOPE, BF_SCOPE_DECL =========================================================================================

#define BF_SCOPE(prvalueExpr)			if (auto BF_DUMMY{prvalueExpr}; false) {} else
#define BF_SCOPE_DECL(declaration)		if (declaration; false) {} else


// === Bad values ======================================================================================================

namespace BF {
	constexpr UInt16  BadValue16  = 0xBAAD;
	constexpr UInt32  BadValue32  = 0xBAADBAAD;
	constexpr UInt64  BadValue64  = 0xBAADBAADBAADBAAD;
	constexpr UIntPtr BadValuePtr = static_cast<UIntPtr>(BadValue64);
}


// === Bad =============================================================================================================

namespace BF {
	struct BadSelector final { explicit BadSelector() = default; };
	constexpr BadSelector Bad;
}


// === Uninitialized ===================================================================================================

namespace BF {
	struct UninitializedSelector final { explicit UninitializedSelector() = default; };
	constexpr UninitializedSelector Uninitialized;
}
