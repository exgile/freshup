#pragma once

#define MAX_PLAYER 2000

struct pc;
class Session;
class Packet;

class PC_Manager {
private:
	pc* pc_list[MAX_PLAYER] = {0};
public:
	PC_Manager();
	~PC_Manager();
	pc* pc_new(Session* session);
	void pc_remove(pc* pc);
	void broadcast(Packet* packet);

	void kickall();
};

extern PC_Manager* pcm;