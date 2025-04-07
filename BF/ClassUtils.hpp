// Utilities for writing classes.


#pragma once


namespace BF {


// === MoveOnlyClass ===================================================================================================

struct MoveOnlyClass {
	MoveOnlyClass() = default;

	MoveOnlyClass(const MoveOnlyClass&) = delete;
	MoveOnlyClass(MoveOnlyClass&&) = default;

	MoveOnlyClass& operator=(const MoveOnlyClass&) = delete;
	MoveOnlyClass& operator=(MoveOnlyClass&&) = default;
};


// === ImmobileClass ===================================================================================================

struct ImmobileClass {
	ImmobileClass() = default;

	ImmobileClass(const ImmobileClass&) = delete;
	ImmobileClass(ImmobileClass&&) = delete;

	ImmobileClass& operator=(const ImmobileClass&) = delete;
	ImmobileClass& operator=(ImmobileClass&&) = delete;
};


}	// namespace BF
