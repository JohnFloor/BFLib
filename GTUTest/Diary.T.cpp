#include "GTU/Diary.hpp"

#include <thread>
#include <type_traits>
#include "gtest/gtest.h"


TEST(Diary, Push)
{
	GTU_XD("")   {}
	GTU_XD("a")  { GTU::Push('a'); }
	GTU_XD("ab") { GTU::Push('a'); GTU::Push('b'); }
	GTU_XD("")   { GTU::Push(""); }
	GTU_XD("ab") { GTU::Push("ab"); }
}


TEST(Diary, Basics)
{
	static_assert(std::is_empty_v<GTU::Diary>);

	{ GTU::Diary s;    GTU_XD("+-") { GTU::Diary d; }                }
	{ GTU::Diary s;    GTU_XD("C-") { GTU::Diary t = s; }            }
	{ GTU::Diary s;    GTU_XD("M-") { GTU::Diary t = std::move(s); } }
	{ GTU::Diary s, t; GTU_XD("C")  { t = s; }                       }
	{ GTU::Diary s, t; GTU_XD("M")  { t = std::move(s); }            }
	{ GTU::Diary d;    GTU_XD("C")  { d = d; }                       }	// self-copy
	{ GTU::Diary d;    GTU_XD("M")  { d = std::move(d); }            }	// self-move
}


TEST(Diary, NestingIsNotSupported)
{
	GTU_XD("-") {			// not "+-" (the library doesn't define what you can expect here)
		GTU::Diary d;
		GTU_XD("") {}
	}
}


TEST(Diary, Parallel)
{
	const auto worker = [] {
		GTU_XD("") {}
	};

	GTU_XD("+-") {
		GTU::Diary s;
		std::thread(worker).join();
	}
}
