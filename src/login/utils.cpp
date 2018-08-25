#include "utils.h"
#include <random>

namespace utils {
	int random_int(int min, int max) {
		std::random_device rd;
		std::mt19937 mt(rd());
		std::uniform_int_distribution<std::mt19937::result_type> dist(min, max);
		return dist(mt);
	}

	std::string random_string(int max_length, std::string possible_chars) {
		std::random_device rd;
		std::mt19937 engine(rd());
		std::uniform_int_distribution<> dist(0, possible_chars.size() - 1);
		std::string ret = "";
		for (int i = 0; i < max_length; i++) {
			int random_index = dist(engine);
			ret += possible_chars[random_index];
		}
		return ret;
	}
}