#pragma once
#include "typedef.h"
#include <vector>
#include <memory>

#define NAME_MAX 40
#define DATE_MAX 16

const int CARD_MAX_PROB = 500;
const int CARD_PROB[4] = { CARD_MAX_PROB, 50, 30, 10 };
const int CARD_PACK1 = 2092957696;
const int CARD_PACK2 = 2092957700;
const int CARD_PACK3 = 2092957701;
const int CARD_PACK4 = 2092957703;
const int CARD_GRANDPRIX = 2092957704;
const int CARD_FRESHUP = 2092957706;

enum {
	TYPE_INVALID = -1,
	TYPE_PANG = 0,
	TYPE_PANG2 = 2,
	TYPE_COOKIE = 1
};

enum {
	ITEMDB_NORMAL = 0,
	ITEMDB_PERIOD = 32,
	ITEMDB_SKIN_PERIOD = 33,
	ITEMDB_PART_RENT = 96,
	ITEMDB_PART_RENT_END = 98
};

enum item_type_name {
	ITEMDB_CHAR = 1,
	ITEMDB_PART = 2,
	ITEMDB_CLUB = 4,
	ITEMDB_BALL = 5,
	ITEMDB_USE = 6,
	ITEMDB_CADDIE = 7,
	ITEMDB_CADDIE_ITEM = 8,
	ITEMDB_SETITEM = 9,
	ITEMDB_SKIN = 14,
	ITEMDB_MASCOT = 16,
	ITEMDB_AUX = 28,
	ITEMDB_CARD = 31,
};

struct itemdb_base {
	uint32 enable;
	uint32 item_typeid;
	char name[NAME_MAX];
	uint8 minlv;
	char preview[NAME_MAX];
	char un[3];
	uint32 price;
	uint32 discount_price;
	uint32 true_price;
	uint8 price_type;
	uint8 flag;
	uint8 time_flag;
	uint8 timing;
	uint32 tp_item;
	uint32 tp_count;
	uint16 mileage;
	uint16 bonusMileage;
	uint16 mileage1;
	uint16 mileage2;
	uint32 tiki_point;
	uint32 tiki_pang;
	uint32 activedate;
	char date_start[DATE_MAX];
	char date_end[DATE_MAX];
};

struct itemdb_normal {
	struct itemdb_base base;
	uint32 type;
	char mpet[NAME_MAX];
	uint16 status[5];
};

struct itemdb_part {
	struct itemdb_base base;
	char mpet[NAME_MAX];
	uint32 ucctype;
	uint32 slot_total;
	uint32 un1; // ?
	char texture[6][NAME_MAX];
	uint16 status[5];
	uint16 slot[5];
	char un2[48];
	uint32 un3, un4;
	uint32 rent_price;
	uint32 un5;
};

struct itemdb_club {
	struct itemdb_base base;
	uint32 club_typeid[4];
	uint16 status[5];
	uint16 max_status[5];
	uint32 club_type;
	uint32 club_special_status;
	uint32 recovery_limit;
	float rate_workshop;
	uint32 un1;
	uint16 transfer;
	uint16 flag;
	uint32 un2[2];
};


struct itemdb_ball {
	struct itemdb_base base;
	uint32 un1;
	char mpet[NAME_MAX];
	uint32 un2[2];
	char un3[560];
	uint16 status[5];
	uint16 un4;
};

struct itemdb_card {
	struct itemdb_base base;
	uint8 type;
	char mpet[NAME_MAX];
	uint8 un1;
	uint16 status[5];
	uint16 effect;
	uint16 effect_qty;
	char texture[3][NAME_MAX];
	uint16 time;
	uint16 in_vol;
	uint32 position;
	uint32 un2[2];
};

struct magicbox {
	uint32 id;
	uint32 enable;
	uint32 sector;
	uint32 char_typeid;
	uint32 level;
	uint32 un;
	uint32 get_typeid;
	uint32 get_amount;
	uint32 use_item[4];
	uint32 use_amount[4];
	uint32 boxid;
	char name[40];
	char datestart[16];
	char dateend[16];
};

class ItemDB {
private:
	void readdb_normal();
	void readdb_part();
	void readdb_club();
	void readdb_ball();
	void readdb_card();
	void readdb_magicbox();
public:
	ItemDB();

	std::vector<std::shared_ptr<itemdb_normal>> item_;
	std::vector<std::shared_ptr<itemdb_part>> part_;
	std::vector<std::shared_ptr<itemdb_club>> club_;
	std::vector<std::shared_ptr<itemdb_ball>> ball_;
	std::vector<std::shared_ptr<itemdb_card>> card_;
	std::vector<std::shared_ptr<magicbox>> magicbox_;
	
	bool exists(uint32 id);
	bool buyable(uint32 id);
	uint32 get_amount(uint32 id);
	std::pair<char, uint32> get_price(uint32 id);

	std::tuple<bool, uint32, uint32> get_cardpackdata(uint32 id);
	std::vector<uint32> get_cardpack(uint32 id);
};

extern ItemDB* itemdb;