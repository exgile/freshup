#pragma once

#include <vector>
#include <memory>

#include "../common/typedef.h"
#include "../common/utils.h"

#include "gameplay.h"

#define MAX_GAME_LIST 1000

struct pc;
class Packet;
class game;
enum roomErr;

enum GAME_UPDATEACTION {
	gCreate = 1,
	gDestroy = 2,
	gUpdate = 3
};

enum PC_GAMEACTION {
	lbSend = 1,
	lbLeave = 2,
	lbUpdate = 3
};

enum {
	gtStroke = 0,
	gtChatroom = 2,
	gtPractice = 19
};

enum {
	cmdWeather = 15
};

struct serverlist {
	std::string name;
	uint32 id;
	std::string ipaddr;
	uint16 port;
	uint16 imgevent;
	uint16 serverimg;
};

STRUCT_PACK(
struct gamedata {
	uint8 un1;
	uint32 vs_time;
	uint32 match_time;
	uint8 max_player;
	uint8 game_type;
	uint8 hole_total;
	uint8 map;
	uint8 mode;
});

STRUCT_PACK(
struct holepractice {
	uint32 natural;
	uint16 roomname_length;
	char name[0x1b];
	uint16 pwd_length;
	char pwd[8];
	uint32 un10;
});

STRUCT_PACK(
struct holerepeat { 
	uint8 holepos;
	uint32 lockhole;
	uint32 natural;
	uint16 roomname_length;
	char name[0x1b];
	uint16 pwd_length;
	char pwd[8];
	uint32 un10;
});

class Channel {
public:
	uint8 id = 0;
	std::string name;
	uint16 maxplayer;
	std::vector<pc*> pc_list;

	game* game_list[MAX_GAME_LIST] = {0};

	uint16 pc_count() { return (uint16)pc_list.size(); }
	
	void sys_send_pc_list(pc* pc);
	void sys_send_game_list(pc* pc);
	void sys_get_pc_data(pc* pc, Packet* p);
	void sys_send_enter_lobby(pc* pc);
	void sys_send_leave_lobby(pc* pc);
	void sys_send(Packet* packet);
	void sys_verify_pc(pc* pc);
	void sys_veriy_game(game* game);
	game* sys_getgame_byid(uint32 room_id);

	void sys_game_action(game* game, GAME_UPDATEACTION const& action);
	void sys_pc_action(pc* pc, PC_GAMEACTION const& action);
	void sys_gm_command(pc* pc);
	int acquire_gameid();

	void send_pc_leave_game(pc* pc);
	void send_room_error(pc* pc, roomErr err);

	void pc_enter_lobby(pc* pc);
	void pc_leave_lobby(pc* pc);
	void pc_quit_lobby(pc* pc);
	void pc_req_chat(pc* pc);

	void pc_req_create_game(pc* pc);
	void pc_req_join_game(pc* pc);

	void game_destroy();

	Channel();
};

class ChannelManager {
public:
	std::vector<Channel*> channel_list;
	std::vector<std::unique_ptr<serverlist>> serverlist_cache;
	ChannelManager();
	~ChannelManager();

	Poco::DateTime lastcache;

	void pc_req_enter_channel(pc* pc);

	Channel* get_channel_ById(uint8 id);

	void getserver_data(pc* pc);
	void send_channel(pc* pc);
	void send_channel_full(pc* pc);
	void send_success(pc* pc);
};

extern ChannelManager* chm;