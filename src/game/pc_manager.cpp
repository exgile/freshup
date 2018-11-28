#include "pc_manager.h"
#include "pc.h"

PC_Manager* pcm = nullptr;

PC_Manager::PC_Manager(){}

PC_Manager::~PC_Manager() {}

void PC_Manager::kickall() {
	for (int p = 0; p < MAX_PLAYER; ++p) {
		if (pc_list[p] != nullptr) {
			pc_list[p]->disconnect();
		}
	}
}

pc* PC_Manager::pc_new(Session* session) {
	int index = -1;

	// get pc free slot index
	for (int i = 0; i < MAX_PLAYER; ++i) {
		if (pc_list[i] == nullptr) {
			index = i;
			break;
		}
	}

	if (index == -1)
		return nullptr;

	pc_list[index] = new pc(index, session);
	return pc_list[index];
}

void PC_Manager::pc_remove(pc* pc) {
	int index = pc->connection_id_;
	if (pc_list[index] == pc) {
		NULL_POINTER(pc_list[index]);
	}
}

void PC_Manager::broadcast(Packet* packet) {
	for (int p = 0; p < MAX_PLAYER; p++) {
		if (pc_list[p] != nullptr) {
			pc_list[p]->send(packet);
		}
	}
}