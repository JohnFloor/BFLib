#include "BF/BasicMath.hpp"


// === IPow() ==========================================================================================================

static_assert(BF::IPow( 0, 0) ==    1);
static_assert(BF::IPow( 0, 1) ==    0);
static_assert(BF::IPow( 0, 2) ==    0);
static_assert(BF::IPow( 0, 3) ==    0);
static_assert(BF::IPow( 0, 4) ==    0);
static_assert(BF::IPow( 0, 5) ==    0);
static_assert(BF::IPow( 0, 6) ==    0);
static_assert(BF::IPow( 0, 7) ==    0);
static_assert(BF::IPow( 0, 8) ==    0);
static_assert(BF::IPow( 0, 9) ==    0);

static_assert(BF::IPow( 1, 0) ==    1);
static_assert(BF::IPow( 1, 1) ==    1);
static_assert(BF::IPow( 1, 2) ==    1);
static_assert(BF::IPow( 1, 3) ==    1);
static_assert(BF::IPow( 1, 4) ==    1);
static_assert(BF::IPow( 1, 5) ==    1);
static_assert(BF::IPow( 1, 6) ==    1);
static_assert(BF::IPow( 1, 7) ==    1);
static_assert(BF::IPow( 1, 8) ==    1);
static_assert(BF::IPow( 1, 9) ==    1);

static_assert(BF::IPow( 2, 0) ==    1);
static_assert(BF::IPow( 2, 1) ==    2);
static_assert(BF::IPow( 2, 2) ==    4);
static_assert(BF::IPow( 2, 3) ==    8);
static_assert(BF::IPow( 2, 4) ==   16);
static_assert(BF::IPow( 2, 5) ==   32);
static_assert(BF::IPow( 2, 6) ==   64);
static_assert(BF::IPow( 2, 7) ==  128);
static_assert(BF::IPow( 2, 8) ==  256);
static_assert(BF::IPow( 2, 9) ==  512);

static_assert(BF::IPow(-1, 0) ==    1);
static_assert(BF::IPow(-1, 1) ==   -1);
static_assert(BF::IPow(-1, 2) ==    1);
static_assert(BF::IPow(-1, 3) ==   -1);
static_assert(BF::IPow(-1, 4) ==    1);
static_assert(BF::IPow(-1, 5) ==   -1);
static_assert(BF::IPow(-1, 6) ==    1);
static_assert(BF::IPow(-1, 7) ==   -1);
static_assert(BF::IPow(-1, 8) ==    1);
static_assert(BF::IPow(-1, 9) ==   -1);

static_assert(BF::IPow(-2, 0) ==    1);
static_assert(BF::IPow(-2, 1) ==   -2);
static_assert(BF::IPow(-2, 2) ==    4);
static_assert(BF::IPow(-2, 3) ==   -8);
static_assert(BF::IPow(-2, 4) ==   16);
static_assert(BF::IPow(-2, 5) ==  -32);
static_assert(BF::IPow(-2, 6) ==   64);
static_assert(BF::IPow(-2, 7) == -128);
static_assert(BF::IPow(-2, 8) ==  256);
static_assert(BF::IPow(-2, 9) == -512);

static_assert(BF::IPow(1, MaxUInt64) == 1);
static_assert(BF::IPow(256llu, 7) == 256llu * 256 * 256 * 256 * 256 * 256 * 256);
// static_assert(BF::IPow(false, 1) == false);		// [CompilationError]: 'Type' should be an integer.
