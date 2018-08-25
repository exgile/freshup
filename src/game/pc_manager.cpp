#include "pc_manager.h"
#include "pc.h"
#include "../common/packet.h"

pc_manager* pcs = nullptr;

pc_manager::pc_manager(){}
pc_manager::~pc_manager() {}

void pc_manager::pc_add(pc* pc) {
	pc_list_.push_back(pc);
}

void pc_manager::pc_remove(pc* pc) {
	auto it = std::find(pc_list_.begin(), pc_list_.end(), pc);
	if (it != pc_list_.end()) {
		pc_list_.erase(it);
	}
}

void pc_manager::broadcast(Packet* packet) {
	for (auto it = pc_list_.begin(); it != pc_list_.end(); ++it) {
		if (*it) {
			(*it)->send_packet(packet);
		}
	}
}