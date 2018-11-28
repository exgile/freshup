#pragma once
#include <string>
#include "session.h"
#include "inventory.h"
#include "exception.h"

#include "../common/utils.h"

class Packet;
class Channel;
class game;

#pragma pack(push, 1)
struct Pos3D {
	float x;
	float y;
	float z;
	void operator+=(const Pos3D& other) {
		x += other.x;
		y += other.y;
		z = other.z;
	}
	void init() {
		x = 0;
		y = 0;
		z = 0;
	}
};

struct statistics {
	uint32 drive;
	uint32 putt;
	uint32 playertime;
	uint32 shottime;
	float longestdistance;
	uint32 pangya;
	uint32 timeout;
	uint32 ob;
	uint32 distancetotal;
	uint32 hole;
	uint32 teamhole;
	uint32 holeinone;
	uint16 bunker;
	uint32 fairway;
	uint32 albratoss;
	uint32 holein;
	uint32 puttin;
	float longestputt;
	float longestchip;
	uint32 exp;
	uint8 level;
	uint64 pang;
	uint32 totalscore;
	uint8 score[5];
	uint8 un1 = 0;
	uint64 maxpang[5];
	uint64 sumpang;
	uint32 gameplayed;
	uint32 disconnected;
	uint32 teamwin;
	uint32 teamgame;
	uint32 ladderpoint;
	uint32 ladderwin;
	uint32 ladderlose;
	uint32 ladderdraw;
	uint32 ladderhole;
	uint32 combocount;
	uint32 maxcombo;
	uint32 nomannergamecount;
	uint64 skinspang;
	uint32 skinswin;
	uint32 skinslose;
	uint32 skinsrunhole;
	uint32 skinsstrikepoint;
	uint32 skinsallincount;
	uint8 un2[6];
	uint32 gamecountseason;
	uint8 un3[8];
};
#pragma pack(pop)

struct pc {
	int recv_length_;
	int recv_pos_;
	Session *session_;
	std::string ip_;

	int connection_id_;
	int account_id_;

	std::string login_key;
	std::string game_key;
	std::string username_;
	std::string name_;
	int sex_;
	int capability_ = 0;
	uint32 cookie;

	// Item Container
	std::vector<PC_ITEM> inventory;
	std::unordered_map<uint32, std::shared_ptr<Club_Data>> club_data_;
	std::shared_ptr<PC_Equipment> equipment;
	std::vector<std::pair<uint8, ITEM_TRANSACTION>> transaction;

	uint32 itemuse[10];

	// Statistic
	std::unique_ptr<statistics> state;

	Channel* channel_;
	bool channel_in_ = false;

	game* game;
	__int16 game_id = -1;
	uint8 game_role = 1;
	uint8 game_slot;
	bool game_ready = false;
	Pos3D game_position;
	uint32 posture = 0;
	uint32 animate = 0;

	// game play variables
	uint8 gameplay_parcount = 0;
	uint8 gameplay_holepos = 0;
	uint32 gameplay_pang = 0;
	uint32 gameplay_bonuspang = 0;


	pc(int con_id, Session *session);
	~pc();
	int get_connection_id();
	void disconnect();
	void skip(int amount);
	void send(Packet *packet);
	void send_packet_undecrypt(Packet *packet);
	void handle_packet(unsigned short bytes_recv);
	void gamedata(Packet* p, bool with_equip = false);

	void transaction_push(uint8 key, ITEM_TRANSACTION tran);
	void transaction_sync();

	bool removepang(int amount);
	bool removecookie(int amount);

	void sendpang();
	void sendcookie();

	void change_equipment();

	void load_statistics();

	template<typename TYPE> TYPE read() {
		if ((recv_length_ - recv_pos_) < sizeof TYPE) throw ReadPacketError();
		TYPE val;
		memcpy(&val, (session_->get_receive_buffer() + recv_pos_), sizeof TYPE);
		recv_pos_ += sizeof TYPE;
		return val;
	}

	template<> bool read<bool>() {
		return read<signed char>() != 0;
	}

	void read(char* data, std::size_t size) {
		if ((recv_length_ - recv_pos_) < size) throw ReadPacketError();
		memcpy(data, (session_->get_receive_buffer() + recv_pos_), size);
		recv_pos_ += size;
	}

	bool readstruct(char* data, std::size_t size) { // Read data into struct, class with fix size
		if ((recv_length_ - recv_pos_) != size) return false;
		memcpy(data, (session_->get_receive_buffer() + recv_pos_), size);
		recv_pos_ += size;
		return true;
	}

	template<> std::string read<std::string>() {
		int len = read<short>();

		if (recv_length_ <= recv_pos_)
			return "";

		if (len > (recv_length_ - recv_pos_)) { return ""; }
		std::string s((char*)session_->get_receive_buffer() + recv_pos_, len);
		recv_pos_ += len;
		return s;
	}
};

enum {
	eqcaddie = 1,
	eqclub = 3,
	eqChar = 4,
	eqmascot = 5,
	eqstart = 7
};