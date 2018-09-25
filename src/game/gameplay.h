#pragma once

#include <string>
#include <vector>

#include "../common/typedef.h"

class pc;
class channel;
struct gamedata;
class Packet;

class game {
public:
	std::vector<pc*> pc_list;

	uint32 owner_uid;
	uint16 roomId;
	uint32 vs_time;
	uint32 match_time;
	uint8 maxplayer;
	uint8 game_type;
	uint8 hole_total;
	uint8 map;
	uint8 mode;
	bool started = false;

	std::string name;
	std::string password;

	char gameKey[16];

	void genkey();
	void send(Packet* p);
	void gameupdate();
	void roomdata(pc* pc);

	virtual void send_pc_created(pc* pc) = 0;

	channel* channel = nullptr;
	game(std::shared_ptr<gamedata> const& data, uint16 room_id, std::string const& room_name, std::string const& room_pwd);
	void addmaster(pc* pc);
};

class game_chatroom : public game {
public:
	void send_pc_created(pc* pc);

	game_chatroom(std::shared_ptr<gamedata> const& data, uint16 room_id, std::string const& room_name, std::string const& room_pwd);
};