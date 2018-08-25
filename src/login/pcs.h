#pragma once
#include <vector>

class pc;
class Session;
class Packet;

class pc_manager {
private:
	std::vector<pc*> pc_list_;
public:
	pc_manager();
	~pc_manager();
	void pc_add(pc* pc);
	void pc_remove(pc* pc);
	void broadcast(Packet* packet);
};

extern pc_manager* pcs;