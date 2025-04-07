#include "BF/ClassUtils.hpp"

#include <type_traits>


// === MoveOnlyClass ===================================================================================================

static_assert( std::is_trivially_default_constructible_v<BF::MoveOnlyClass>);

static_assert(!std::is_constructible_v<BF::MoveOnlyClass, BF::MoveOnlyClass&>);
static_assert(!std::is_constructible_v<BF::MoveOnlyClass, const BF::MoveOnlyClass&>);
static_assert( std::is_trivially_constructible_v<BF::MoveOnlyClass, BF::MoveOnlyClass&&>);
static_assert(!std::is_constructible_v<BF::MoveOnlyClass, const BF::MoveOnlyClass&&>);

static_assert(!std::is_assignable_v<BF::MoveOnlyClass&, BF::MoveOnlyClass&>);
static_assert(!std::is_assignable_v<BF::MoveOnlyClass&, const BF::MoveOnlyClass&>);
static_assert( std::is_trivially_assignable_v<BF::MoveOnlyClass&, BF::MoveOnlyClass&&>);
static_assert(!std::is_assignable_v<BF::MoveOnlyClass&, const BF::MoveOnlyClass&&>);

static_assert( std::is_trivially_destructible_v<BF::MoveOnlyClass>);


// === ImmobileClass ===================================================================================================

static_assert( std::is_trivially_default_constructible_v<BF::ImmobileClass>);

static_assert(!std::is_constructible_v<BF::ImmobileClass, BF::ImmobileClass&>);
static_assert(!std::is_constructible_v<BF::ImmobileClass, const BF::ImmobileClass&>);
static_assert(!std::is_constructible_v<BF::ImmobileClass, BF::ImmobileClass&&>);
static_assert(!std::is_constructible_v<BF::ImmobileClass, const BF::ImmobileClass&&>);

static_assert(!std::is_assignable_v<BF::ImmobileClass&, BF::ImmobileClass&>);
static_assert(!std::is_assignable_v<BF::ImmobileClass&, const BF::ImmobileClass&>);
static_assert(!std::is_assignable_v<BF::ImmobileClass&, BF::ImmobileClass&&>);
static_assert(!std::is_assignable_v<BF::ImmobileClass&, const BF::ImmobileClass&&>);

static_assert( std::is_trivially_destructible_v<BF::ImmobileClass>);
