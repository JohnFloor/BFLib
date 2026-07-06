#include "GTU/Diary.hpp"

#include <chrono>
#include <thread>
#include <type_traits>
#include <vector>
#include "gtest/gtest.h"
using namespace std::chrono_literals;


// === Helpers =========================================================================================================

static void Work()
{
	GTU_XD("+CC--") {
		GTU::Diary s;
		std::this_thread::sleep_for(5ms);
		GTU::Diary t = s;
		std::this_thread::sleep_for(5ms);
		t = s;
	}
}


static void Worker(size_t nWork)
{
	for (size_t i = 0; i < nWork; i++)
		Work();
}


// === Tests ===========================================================================================================
TEST(Diary, Basics)
{
	static_assert(std::is_empty_v<GTU::Diary>);

	{                  GTU_XD("")   {}                               }
	{ GTU::Diary s;    GTU_XD("+-") { GTU::Diary d; }                }
	{ GTU::Diary s;    GTU_XD("C-") { GTU::Diary t = s; }            }
	{ GTU::Diary s;    GTU_XD("M-") { GTU::Diary t = std::move(s); } }
	{ GTU::Diary s, t; GTU_XD("C")  { t = s; }                       }
	{ GTU::Diary s, t; GTU_XD("M")  { t = std::move(s); }            }
}


TEST(Diary, Parallel)
{
	constexpr size_t NThreads = 10;
	constexpr size_t NWork    = 10;

	std::vector<std::thread> threads;

	for (size_t i = 0; i < NThreads; i++)
		threads.emplace_back(&Worker, NWork);

	for (std::thread& th : threads)
		th.join();
}
