#pragma once

#include <vector>
#include <memory>
#include "typedef.h"

class pc;
class Packet;

class Channel {
public:
	uint8 id = 0;
	std::string name;
	uint16 maxplayer;
	std::vector<pc*> pc_list;

	uint16 pc_count() { return (uint16)pc_list.size(); }
	
	void sys_send_pc_list(pc* pc);
	void sys_send_this_pc(pc* pc, enum pc_send_type a);
	void sys_get_pc_data(pc* pc, Packet* packet);
	void sys_send_pc_message(pc* pc, std::string& message);
	void sys_send_enter_lobby(pc* pc);
	void sys_send_leave_lobby(pc* pc);
	void sys_send(Packet& packet);
	void sys_verify_pc(pc* pc);

	void pc_enter_lobby(pc* pc);
	void pc_leave_lobby(pc* pc);
	void pc_quit_lobby(pc* pc);
	void pc_send_message(pc* pc);
};

class ChannelManager {
public:
	std::vector<Channel*> channel_list;
	ChannelManager();
	~ChannelManager();

	void pc_select_channel(pc* pc);

	Channel* get_channel_ById(uint8 id);

	void send_channel(pc* pc);
	void send_channel_full(pc* pc);
	void send_success(pc* pc);
};

 enum pc_send_type {
	pc_show_lobby = 1,
	pc_leave_lobby = 2,
	pc_update_lobby = 3
};

extern ChannelManager* channel_manager;