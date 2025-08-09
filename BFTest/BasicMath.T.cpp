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


// === IsPowerOf2() ====================================================================================================

static_assert(!BF::IsPowerOf2(6));
static_assert(!BF::IsPowerOf2(24));
static_assert(!BF::IsPowerOf2(255));
static_assert(!BF::IsPowerOf2(32767));
static_assert(!BF::IsPowerOf2(65535));
static_assert(!BF::IsPowerOf2(0b111110000));

static_assert(!BF::IsPowerOf2(0));

static_assert( BF::IsPowerOf2(1llu <<  0));
static_assert( BF::IsPowerOf2(1llu <<  1));
static_assert( BF::IsPowerOf2(1llu <<  2));
static_assert( BF::IsPowerOf2(1llu <<  3));
static_assert( BF::IsPowerOf2(1llu <<  4));
static_assert( BF::IsPowerOf2(1llu <<  5));
static_assert( BF::IsPowerOf2(1llu <<  6));
static_assert( BF::IsPowerOf2(1llu <<  7));
static_assert( BF::IsPowerOf2(1llu <<  8));
static_assert( BF::IsPowerOf2(1llu <<  9));

static_assert( BF::IsPowerOf2(1llu << 10));
static_assert( BF::IsPowerOf2(1llu << 11));
static_assert( BF::IsPowerOf2(1llu << 12));
static_assert( BF::IsPowerOf2(1llu << 13));
static_assert( BF::IsPowerOf2(1llu << 14));
static_assert( BF::IsPowerOf2(1llu << 15));
static_assert( BF::IsPowerOf2(1llu << 16));
static_assert( BF::IsPowerOf2(1llu << 17));
static_assert( BF::IsPowerOf2(1llu << 18));
static_assert( BF::IsPowerOf2(1llu << 19));

static_assert( BF::IsPowerOf2(1llu << 20));
static_assert( BF::IsPowerOf2(1llu << 21));
static_assert( BF::IsPowerOf2(1llu << 22));
static_assert( BF::IsPowerOf2(1llu << 23));
static_assert( BF::IsPowerOf2(1llu << 24));
static_assert( BF::IsPowerOf2(1llu << 25));
static_assert( BF::IsPowerOf2(1llu << 26));
static_assert( BF::IsPowerOf2(1llu << 27));
static_assert( BF::IsPowerOf2(1llu << 28));
static_assert( BF::IsPowerOf2(1llu << 29));

static_assert( BF::IsPowerOf2(1llu << 30));
static_assert( BF::IsPowerOf2(1llu << 31));
static_assert( BF::IsPowerOf2(1llu << 32));
static_assert( BF::IsPowerOf2(1llu << 33));
static_assert( BF::IsPowerOf2(1llu << 34));
static_assert( BF::IsPowerOf2(1llu << 35));
static_assert( BF::IsPowerOf2(1llu << 36));
static_assert( BF::IsPowerOf2(1llu << 37));
static_assert( BF::IsPowerOf2(1llu << 38));
static_assert( BF::IsPowerOf2(1llu << 39));

static_assert( BF::IsPowerOf2(1llu << 40));
static_assert( BF::IsPowerOf2(1llu << 41));
static_assert( BF::IsPowerOf2(1llu << 42));
static_assert( BF::IsPowerOf2(1llu << 43));
static_assert( BF::IsPowerOf2(1llu << 44));
static_assert( BF::IsPowerOf2(1llu << 45));
static_assert( BF::IsPowerOf2(1llu << 46));
static_assert( BF::IsPowerOf2(1llu << 47));
static_assert( BF::IsPowerOf2(1llu << 48));
static_assert( BF::IsPowerOf2(1llu << 49));

static_assert( BF::IsPowerOf2(1llu << 50));
static_assert( BF::IsPowerOf2(1llu << 51));
static_assert( BF::IsPowerOf2(1llu << 52));
static_assert( BF::IsPowerOf2(1llu << 53));
static_assert( BF::IsPowerOf2(1llu << 54));
static_assert( BF::IsPowerOf2(1llu << 55));
static_assert( BF::IsPowerOf2(1llu << 56));
static_assert( BF::IsPowerOf2(1llu << 57));
static_assert( BF::IsPowerOf2(1llu << 58));
static_assert( BF::IsPowerOf2(1llu << 59));

static_assert( BF::IsPowerOf2(1llu << 60));
static_assert( BF::IsPowerOf2(1llu << 61));
static_assert( BF::IsPowerOf2(1llu << 62));
static_assert( BF::IsPowerOf2(1llu << 63));
