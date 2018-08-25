#pragma once
#include <vector>
#include <mutex>

class unique_id {
private:
	std::vector<int> vector_int;
	std::mutex safe;

public:
	// class constructor
	unique_id();

	int get();
	void store(int con_id);
};

extern unique_id* unique_s;