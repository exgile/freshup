#include "unique.h"
#include "consts.h"

unique_id* unique_s = nullptr;

unique_id::unique_id() {
	for (int i = MAX_PLAYER; i >= 1; --i) {
		vector_int.push_back(i);
	}
}

int unique_id::get() {
	std::lock_guard<std::mutex> lock(safe);

	int con = vector_int.back();
	vector_int.pop_back();

	return con;
}

void unique_id::store(int con_id) {
	std::lock_guard<std::mutex> lock(safe);

	if (!(std::find(vector_int.begin(), vector_int.end(), con_id) != vector_int.end()))
		vector_int.push_back(con_id);
}