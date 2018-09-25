#pragma once
#include "typedef.h"

#include <random>

#define STRUCT_PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop) )

#define COUNTOF(x) (sizeof(x)/sizeof(*x))

namespace utils {
	int random_int(int min, int max);
	std::string random_string(int max_length, std::string possible_chars = "abcdef1234567890");
	uint8 itemdb_type(uint32 id);
}