#pragma once

#include <string>
#include <vector>

#include "../common/typedef.h"

struct pc;
class Channel;
struct gamedata;
class Packet;

struct holepractice;
struct holerepeat;

#pragma pack(push, 1)
struct matchdata {
	uint32 connection_id;
	float x;
	float y;
	float z;
	uint8 shottype;
	char un1[2];
	uint32 pang;
	uint32 bonuspang;
	char un2[4];
	char matchinfomation[6];
	char un3[17];
};
#pragma pack(pop)

struct holedata {
	uint8 holenum;
	uint8 weather;
	uint16 windpower;
	uint16 winddirection;
	uint8 map;
	uint8 position;
};

enum practicetype {
	tNone = 0,
	tPractice = 1,
	tRepeat = 2
};

class game {
public:
	std::vector<pc*> pc_list;

	bool valid = true;

	uint32 owner_uid;
	uint16 roomId;
	uint32 vs_time;
	uint32 match_time;
	uint8 maxplayer;
	uint8 game_type;
	uint8 hole_total;
	uint8 map;
	uint8 mode;
	uint32 natural = 0;
	bool started = false;

	std::array<holedata, 18> holes;

	practicetype practicetype = tNone;
	uint8 practice_holeposition;
	uint32 practice_lockhole;

	std::string name;
	std::string password;

	char gameKey[16];

	void sys_verify_pc(pc *pc);
	void sys_shotdecrypt(char* data, std::size_t size);

	void genkey();
	void send(Packet *p);
	void roomdata(pc *pc);
	void roomdata(Packet *p);

	virtual void gameupdate();
	virtual void send_pc_create(pc *pc) = 0;
	virtual void send_pc_join(pc *pc) = 0;
	virtual void sys_calc_pcslot();
	virtual void startgame();
	virtual void hole_init();

	bool pc_remove(pc *pc);

	void sys_send_pcleave(pc *pc);
	void sys_weather(uint8 weather);

	Channel *channel = nullptr;
	game(std::shared_ptr<gamedata> const& data, uint16 room_id, std::string const& room_name, std::string const& room_pwd);

	virtual void addmaster(pc *pc);
	void addpc(pc *pc);

	void pc_action(pc *pc);
	void pc_change_game_config(pc *pc);
	void pc_chat(pc *pc, Packet *p);
	void pc_req_holesync(pc *pc);
	void pc_req_sync_shotdata(pc *pc);
	virtual void pc_loadmap_success(pc *pc) = 0;
	virtual void pc_req_gamedata(pc *pc) = 0;
	virtual void pc_req_leave_game(pc *pc) = 0;
};

class game_chatroom : public game {
public:
	void send_pc_create(pc *pc);
	void send_pc_join(pc *pc);
	void pc_req_gamedata(pc *pc);
	void pc_loadmap_success(pc *pc);
	void addmaster(pc *pc);
	void pc_req_leave_game(pc *pc);

	game_chatroom(std::shared_ptr<gamedata> const& data, uint16 room_id, std::string const& room_name, std::string const& room_pwd);
};

class game_stroke : public game {
public:
	void send_pc_create(pc *pc);
	void send_pc_join(pc *pc);
	void pc_req_gamedata(pc *pc);
	void pc_loadmap_success(pc *pc);
	void addmaster(pc *pc);
	void pc_req_leave_game(pc *pc);

	game_stroke(std::shared_ptr<gamedata> const& data, uint16 room_id, std::string const& room_name, std::string const& room_pwd);
};

class game_practice : public game {
public:
	void send_pc_create(pc *pc);
	void send_pc_join(pc *pc);
	void pc_req_gamedata(pc *pc);
	void gameupdate();
	void startgame();
	void pc_loadmap_success(pc *pc);
	void pc_req_leave_game(pc *pc);

	game_practice(std::shared_ptr<gamedata> const& data, holepractice& practicedata, uint16 room_id);
	game_practice(std::shared_ptr<gamedata> const& data, holerepeat& repeatdata, uint16 room_id);
};

enum rSetting {
	rsName = 0,
	rsPwd = 1,
	rsMap = 3,
	rsMode = 5,
	rsNatural = 14
};

enum roomErr {
	rFull = 2,
	rNotExist = 3,
	rPwdErr = 4,
	rFail = 7
};

enum rAction {
	rPCMoveAround = 0,
	rPCVSAnimate = 1,
	rPCAppear = 4,
	rPCPosture = 5,
	rPCMove = 6,
	rPCAction = 7,
	rPCAnimation = 8
};