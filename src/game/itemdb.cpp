#include "itemdb.h"

#include "clif.h"
#include "../common/packet.h"
#include "../common/utils.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include <fstream>
#include <sstream> // istringstream
#include <iostream> // DELETE SOON

#include "pc.h" // PC CLASS

ItemDB* itemdb = nullptr;

ItemDB::ItemDB() {
	readdb_normal();
	readdb_part();
	readdb_club();
	readdb_ball();
	readdb_card();
	readdb_set();
	readdb_magicbox();
	read_random_group("config/random.txt");
	read_random_group("config/card.txt");
}

void ItemDB::readdb_normal() {
	std::fstream f;
	int count;

	f.open("db/Item.iff", std::fstream::in | std::fstream::binary);
	f.read((char*)&count, sizeof(count));
	f.seekg(4, std::ios::cur);
	
	for (int i = 1; i <= count; ++i) {
		std::unique_ptr<itemdb_normal> it = CREATE_UNIQUE(itemdb_normal);
		f.read((char*)&it->base.enable, sizeof(itemdb_normal));
		item_data[it->base.item_typeid] = std::move(it);
	}
	f.close();
}

void ItemDB::readdb_part() {
	std::fstream f;
	int count;

	f.open("db/Part.iff", std::fstream::in | std::fstream::binary);
	f.read((char*)&count, sizeof(count));
	f.seekg(4, std::ios::cur);

	for (int i = 1; i <= count; ++i) {
		std::unique_ptr<itemdb_part> it = CREATE_UNIQUE(itemdb_part);
		f.read((char*)&it->base.enable, sizeof(itemdb_part));
		part_data[it->base.item_typeid] = std::move(it);
	}
	f.close();
}

void ItemDB::readdb_set() {
	std::fstream f;
	int count;

	f.open("db/SetItem.iff", std::fstream::in | std::fstream::binary);
	f.read((char*)&count, sizeof(count));
	f.seekg(4, std::ios::cur);

	for (int i = 0; i <= count; ++i) {
		std::unique_ptr<itemdb_set> it = CREATE_UNIQUE(itemdb_set);
		f.read((char*)&it->base.enable, sizeof(itemdb_set));
		set_data[it->base.item_typeid] = std::move(it);
	}
	f.close();
}

void ItemDB::readdb_club() {
	std::fstream f;
	int count;

	f.open("db/ClubSet.iff", std::fstream::in | std::fstream::binary);
	f.read((char*)&count, sizeof(count));
	f.seekg(4, std::ios::cur);

	for (int i = 1; i <= count; ++i) {
		std::unique_ptr<itemdb_club> it = CREATE_UNIQUE(itemdb_club);
		f.read((char*)&it->base.enable, sizeof(itemdb_club));
		club_data[it->base.item_typeid] = std::move(it);
	}

	f.close();
}

void ItemDB::readdb_ball() {
	std::fstream f;
	int count;

	f.open("db/Ball.iff", std::fstream::in | std::fstream::binary);
	f.read((char*)&count, sizeof(count));
	f.seekg(4, std::ios::cur);

	for (int i = 1; i <= count; ++i) {
		std::unique_ptr<itemdb_ball> it = CREATE_UNIQUE(itemdb_ball);
		f.read((char*)&it->base.enable, sizeof(itemdb_ball));
		ball_data[it->base.item_typeid] = std::move(it);
	}
	f.close();
}

void ItemDB::readdb_card() {
	std::fstream f;

	std::ofstream myfile;
	myfile.open("example.txt");

	int count;

	f.open("db/Card.iff", std::fstream::in | std::fstream::binary);
	f.read((char*)&count, sizeof(count));
	f.seekg(4, std::ios::cur);

	for (int i = 1; i <= count; ++i) {
		std::unique_ptr<itemdb_card> it = CREATE_UNIQUE(itemdb_card);
		f.read((char*)&it->base.enable, sizeof(itemdb_card));
		if (strlen(it->base.name) <= 0) continue;

		//if (it->in_vol == 4) {
		//if (it->type > 0){
			int rate = 0;

			if (it->type == 0) {
				rate = 100;
			}
			else if (it->type == 1) {
				rate = 10;
			}
			else if (it->type == 2) {
				rate = 5;
			}
			else if (it->type == 3) {
				rate = 3;
			}

			myfile << 2092957704 << "," << it->base.item_typeid << "," << rate << "," << 1 <<  "," << 1 << " // " << it->base.name << "\n";
		//}

		//printf("%d, %s\n" , it->base.item_typeid, it->base.name);
		card_data[it->base.item_typeid] = std::move(it);
	}

	f.close();
	myfile.close();
}

void ItemDB::readdb_magicbox() {
	std::fstream f;
	uint16 count;

	f.open("db/CadieMagicBox.iff", std::fstream::in | std::fstream::binary);
	f.read((char*)&count, sizeof(count));
	f.seekg(6, std::ios::cur);

	for (int i = 1; i <= count; ++i) {
		std::unique_ptr<magicbox> it = CREATE_UNIQUE(magicbox);
		f.read((char*)&it->id, sizeof(magicbox));
		magicbox_data[it->id] = std::move(it);
	}
	f.close();
}

bool ItemDB::exists(uint32 id) {
	uint8 item_type = itemdb_type(id);

	if (item_type == ITEMDB_USE) {
		if (item_data.find(id) != item_data.end()) {
			return true;
		}
	}
	else if (item_type == ITEMDB_PART) {
		if (part_data.find(id) != part_data.end()) {
			return true;
		}
	}
	else if (item_type == ITEMDB_CLUB) {
		if (club_data.find(id) != club_data.end()) {
			return true;
		}
	}
	else if (item_type == ITEMDB_BALL) {
		if (ball_data.find(id) != ball_data.end()) {
			return true;
		}
	}
	else if (item_type == ITEMDB_CARD) {
		if (card_data.find(id) != card_data.end()) {
			return true;
		}
	}
	else if (item_type == ITEMDB_SETITEM) {
		if (set_data.find(id) != set_data.end()) {
			return true;
		}
	}
	return false;
}

uint32 ItemDB::get_amount(uint32 id) {
	uint8 item_type = itemdb_type(id);

	if (item_type == ITEMDB_USE) {
		if (item_data.find(id) != item_data.end()) {
			return item_data[id]->status[0] == 0 ? 1 : item_data[id]->status[0];
		}
	}

	return 1;
}

std::string ItemDB::getname(uint32 id) {
	uint8 item_type = itemdb_type(id);

	if (item_type == ITEMDB_USE) {
		if (item_data.find(id) != item_data.end()) {
			return item_data[id]->base.name;
		}
	}
	else if (item_type == ITEMDB_PART) {
		if (part_data.find(id) != part_data.end()) {
			return part_data[id]->base.name;
		}
	}
	else if (item_type == ITEMDB_CLUB) {
		if (club_data.find(id) != club_data.end()) {
			return club_data[id]->base.name;
		}
	}
	else if (item_type == ITEMDB_BALL) {
		if (ball_data.find(id) != ball_data.end()) {
			return ball_data[id]->base.name;
		}
	}
	else if (item_type == ITEMDB_CARD) {
		if (card_data.find(id) != card_data.end()) {
			return card_data[id]->base.name;
		}
	}
	else if (item_type == ITEMDB_SETITEM) {
		if (set_data.find(id) != set_data.end()) {
			return set_data[id]->base.name;
		}
	}
	return "";
}

std::pair<char, uint32> ItemDB::get_price(uint32 id) {
	uint8 item_type = itemdb_type(id);

	if (item_type == ITEMDB_USE) {
		if (item_data.find(id) != item_data.end()) {
			return std::make_pair(item_data[id]->base.price_type, item_data[id]->base.discount_price > 0 ? item_data[id]->base.discount_price : item_data[id]->base.price);
		}
	}
	else if (item_type == ITEMDB_CARD) {
		if (card_data.find(id) != card_data.end()) {
			return std::make_pair(card_data[id]->base.price_type, card_data[id]->base.discount_price > 0 ? card_data[id]->base.discount_price : card_data[id]->base.price);
		}
	}
	else if (item_type == ITEMDB_SETITEM) {
		if (set_data.find(id) != set_data.end()) {
			return std::make_pair(set_data[id]->base.price_type, set_data[id]->base.discount_price > 0 ? set_data[id]->base.discount_price : set_data[id]->base.price);
		}
	}

	return std::make_pair(TYPE_INVALID, 0);
}

bool ItemDB::buyable(uint32 id) {
	uint8 item_type = itemdb_type(id);

	if (item_type == ITEMDB_USE) {
		if (item_data.find(id) != item_data.end()) {
			return item_data[id]->base.flag & 1;
		}
	}
	else if (item_type == ITEMDB_CARD) {
		if (card_data.find(id) != card_data.end()) {
			return card_data[id]->base.flag & 1;
		}
	}
	else if (item_type == ITEMDB_SETITEM) {
		if (set_data.find(id) != set_data.end()) {
			return set_data[id]->base.flag & 1;
		}
	}

	return false;
}

std::pair<bool, uint32> ItemDB::cardpack_data(uint32 id) {
	switch (id) {
	case 2092957696:
		return std::make_pair(true, 3);
		break;
	case 2092957700:
		return std::make_pair(true, 3);
		break;
	case 2092957701:
		return std::make_pair(true, 3);
		break;
	case 2092957703:
		return std::make_pair(true, 3);
		break;
	case 2092957706:
		return std::make_pair(true, 3);
	case 2092957704:
		return std::make_pair(true, 3);
	}

	return std::make_pair(false, 0);
}

std::vector<uint32> ItemDB::get_cardpack(uint32 id) {
	std::vector<uint32> list;

	// get group id
	auto group = group_db[id];

	if (!group) {
		return list;
	}

	std::pair<bool, uint32> data = cardpack_data(id);

	do {
		for (int i = 0; i < MAX_RAND_GROUP; ++i) {
			uint16 rand;
			if (!(int)group->random[i]->data.size())
				continue;
			rand = (rnd() % (int)group->random[i]->data.size());

			uint32 card_typeid = group->random[i]->data[rand]->id;

			if (  std::find(std::begin(list), std::end(list), card_typeid) == std::end(list) ) {
				list.push_back(card_typeid);
			}
		}
	} while ( (int)list.size() < data.second);

	return list;
}

void ItemDB::pc_use_cardpack(pc* pc) {
	assert(pc);

	uint32 card_typeid = pc->read<uint32>();

	std::vector<uint32> pack_data = get_cardpack(card_typeid);

	if (!(uint8)pack_data.size()) {
		clif_pc_opencard_failed(pc);
		return;
	}

	ITEM_TRANSACTION card_ret = CREATE_SHARED(INV_TRANSACTION);
	char ret = pc_delitem(pc, card_typeid, 1, &card_ret);

	if (ret != DELITEM_SUCCESS) {
		clif_pc_opencard_failed(pc);
		return;
	}

	Packet p;
	WTHEAD(&p, 0x154);
	WTIU32(&p, 0); // SUCCESS ??
	WTIU32(&p, card_ret->item_id);
	WTIU32(&p, card_ret->item_typeid);
	WTZERO(&p, 0x0C);
	WTIU32(&p, 1);
	WTZERO(&p, 0x20);
	WTIU16(&p, 1);
	WTIU08(&p, (uint8)pack_data.size());

	for (int i = 0; i < (uint8)pack_data.size(); ++i) {
		item item;
		item.type_id = pack_data[i];
		item.amount = 1;

		ITEM_TRANSACTION add_ret = CREATE_SHARED(INV_TRANSACTION);

		pc_additem(pc, &item, false, &add_ret);

		WTIU32(&p, add_ret->item_id);
		WTIU32(&p, add_ret->item_typeid);
		WTZERO(&p, 0x0C);
		WTIU32(&p, add_ret->c0);
		WTZERO(&p, 0x20);
		WTIU16(&p, 1);
		WTIU32(&p, 1);
	}
	pc->send(&p);
}

void ItemDB::pc_req_bongdari_normal(pc *pc) {
	auto group = group_db[436207658];

	if (!group)
		return;

	std::vector<std::tuple<uint32, uint16, uint8>> list;

	std::vector<int> weight = { 0, 100, 70, 50, 30 };
	int amount = rnd_weight(weight);

	do {
		for (int i = 0; i < MAX_RAND_GROUP; ++i) {
			uint16 rand;
			if (!(int)group->random[i]->data.size())
				continue;
			rand = (rnd() % (int)group->random[i]->data.size());

			uint32 item_typeid = group->random[i]->data[rand]->id;
			
			auto it = std::find_if(list.begin(), list.end(), [&item_typeid](std::tuple<uint32, uint16, uint8> const &rval) {
				return std::get<0>(rval) == item_typeid;
			});

			if ( it == list.end() ) {
				list.push_back( std::make_tuple(group->random[i]->data[rand]->id, rnd_weight(group->random[i]->data[rand]->rnd_weight), group->random[i]->data[rand]->rare) );
			}
		}
	} while ((int)list.size() < amount);

	if ((int)list.size() >= 1) {
		int stuff = 0;

		ITEM_TRANSACTION tran = CREATE_SHARED(INV_TRANSACTION);

		if (pc_delitem(pc, 436207656, 1, &tran) == DELITEM_SUCCESS) // Bongdari CP (Event)
		{
			stuff = tran->item_id;
			pc->transaction_push(2, tran);
		}
		else if (pc_delitem(pc, 436207657, 1, &tran) == DELITEM_SUCCESS) { // Bongdari CP (Gift)
			stuff = tran->item_id;
			pc->transaction_push(2, tran);
		}
		else if (pc_delitem(pc, 436207658, 1, &tran) == DELITEM_SUCCESS) { // Bongdari CP (GM)
			stuff = tran->item_id;
			pc->transaction_push(2, tran);
		}
		else if (pc->removepang(900)) {
			stuff = 0;
			pc->sendpang();
		}
		else {
			clif_bongdari_not_enoughtmoney(pc);
			return;
		}

		clif_bongdari_normal_begin(pc);

		Packet p;
		WTHEAD(&p, 0x21b);
		WTIU32(&p, 0);
		WTIU32(&p, stuff);
		WTIU32(&p, (uint32)list.size());

		for (auto &it : list)
		{
			item item;
			item.type_id = std::get<0>(it);
			item.amount = std::get<1>(it);

			ITEM_TRANSACTION tran = CREATE_SHARED(INV_TRANSACTION);

			if (pc_additem(pc, &item, false, &tran) == ADDITEM_SUCCESS)
			{
				pc->transaction_push(2, tran);
				WTIU32(&p, rnd() % 3);
				WTIU32(&p, tran->item_typeid);
				WTIU32(&p, tran->item_id);
				WTIU32(&p, item.amount);
				WTIU32(&p, std::get<2>(it));
			}
		}																																													
		
		WTIU64(&p, pc->state->pang);
		WTIU64(&p, pc->cookie);

		pc->transaction_sync();
		pc->send(&p);
	}
}

void ItemDB::pc_req_bongdari_big(pc *pc) {
	auto group = group_db[436207657];

	if (!group)
		return;

	std::vector<std::tuple<uint32, uint16, uint8>> list;

	std::vector<int> weight = { 0, 0, 0, 100, 70, 60, 50, 40, 30 };
	int amount = rnd_weight(weight);

	do {
		for (int i = 0; i < MAX_RAND_GROUP; ++i) {
			uint16 rand;
			if (!(int)group->random[i]->data.size())
				continue;
			rand = (rnd() % (int)group->random[i]->data.size());

			uint32 item_typeid = group->random[i]->data[rand]->id;

			auto it = std::find_if(list.begin(), list.end(), [&item_typeid](std::tuple<uint32, uint16, uint8> const &rval) {
				return std::get<0>(rval) == item_typeid;
			});

			if (it == list.end()) {
				list.push_back(std::make_tuple(group->random[i]->data[rand]->id, rnd_weight(group->random[i]->data[rand]->rnd_weight), group->random[i]->data[rand]->rare));
			}
		}
	} while ((int)list.size() < amount);

	if ((int)list.size() >= 1) {
		if (!pc->removepang(10000)) {
			clif_bongdari_not_enoughtmoney_big(pc);
			return;
		}

		pc->sendpang();

		clif_bongdari_normal_begin(pc);

		Packet p;
		WTHEAD(&p, 0x26c);
		WTIU32(&p, 0);
		WTIU32(&p, 0);
		WTIU32(&p, (uint32)list.size());

		for (auto &it : list) {
			item item;
			item.type_id = std::get<0>(it);
			item.amount = std::get<1>(it);
			ITEM_TRANSACTION tran = CREATE_SHARED(INV_TRANSACTION);

			if (pc_additem(pc, &item, false, &tran) == ADDITEM_SUCCESS) {
				pc->transaction_push(2, tran);
				WTIU32(&p, rnd() % 3);
				WTIU32(&p, tran->item_typeid);
				WTIU32(&p, tran->item_id);
				WTIU32(&p, item.amount);
				WTIU32(&p, std::get<2>(it));
			}
		}

		WTIU64(&p, pc->state->pang);
		WTIU64(&p, pc->cookie);

		pc->transaction_sync();
		pc->send(&p);
	}
}

void ItemDB::pc_req_scratch_play(pc *pc) {
	auto group = group_db[436207664];

	if (!group)
		return;

	std::vector<std::tuple<uint32, uint16, uint8>> list;

	std::vector<int> weight = { 100, 30 };
	int amount = rnd_weight(weight);

	do {
		for (int i = 0; i < MAX_RAND_GROUP; ++i) {
			uint16 rand;
			if (!(int)group->random[i]->data.size())
				continue;
			rand = (rnd() % (int)group->random[i]->data.size());

			uint32 item_typeid = group->random[i]->data[rand]->id;

			auto it = std::find_if(list.begin(), list.end(), [&item_typeid](std::tuple<uint32, uint16, uint8> const &rval) {
				return std::get<0>(rval) == item_typeid;
			});

			if (it == list.end()) {
				list.push_back(std::make_tuple(group->random[i]->data[rand]->id, rnd_weight(group->random[i]->data[rand]->rnd_weight), group->random[i]->data[rand]->rare));
			}
		}
	} while ((int)list.size() < amount);

	if ((int)list.size() >= 1) {
		ITEM_TRANSACTION tran = CREATE_SHARED(INV_TRANSACTION);

		if (pc_delitem(pc, 436207664, 1, &tran) == DELITEM_SUCCESS) // Scratch Card (Gift)
		{
			pc->transaction_push(2, tran);
		}
		else if (pc_delitem(pc, 436207667, 1, &tran) == DELITEM_SUCCESS) // Scratch Card (Event)
		{
			pc->transaction_push(2, tran);
		}
		else if (pc_delitem(pc, 436207668, 1, &tran) == DELITEM_SUCCESS) // Scratch CP (GM)
		{
			pc->transaction_push(2, tran);
		}
		else {
			clif_scratch_not_enough_item(pc);
			return;
		}

		Packet p;
		WTHEAD(&p, 0xDD);
		WTIU32(&p, 0);
		WTIU32(&p, (uint32)list.size());

		for (auto &it : list)
		{
			item item;
			item.type_id = std::get<0>(it);
			item.amount = std::get<1>(it);

			ITEM_TRANSACTION tran = CREATE_SHARED(INV_TRANSACTION);

			if (pc_additem(pc, &item, false, &tran) == ADDITEM_SUCCESS)
			{
				pc->transaction_push(2, tran);
				WTZERO(&p, 4);
				WTIU32(&p, tran->item_typeid);
				WTIU32(&p, tran->item_id);
				WTIU32(&p, item.amount);
				WTIU32(&p, std::get<2>(it));
			}
		}

		pc->transaction_sync();
		pc->send(&p);
	}
}

void ItemDB::read_random_group(const char* path_to_file){
	std::ifstream file(path_to_file);

	if (!file) {
		spdlog::get("console")->error("read_card_random(): cannot read file {}", path_to_file);
		return;
	}

	std::string line;

	while (std::getline(file, line)) {
		// find comment then replace with null
		std::size_t spos = line.find("//");
		if (spos != std::string::npos) {
			line.replace(spos, line.length() - spos, "");
		}
		// skip for empty line
		if (line[0] == '\0' || line[0] == '\n' || line[0] == '\r')
			continue;

		std::string field;
		std::vector<std::string> seperated_fields;
		std::istringstream sfile(line);

		while (std::getline(sfile, field, ',')) {
			seperated_fields.push_back(field);
		}

		{
			int group_id = -1;
			unsigned int j, prob = 1;
			uint8 rand_group = 1;

			std::shared_ptr<item_group_random> random = nullptr;
			std::shared_ptr<item_group_entry> entry = std::make_shared<item_group_entry>();

			group_id = std::stoi(seperated_fields[0]);
			prob = std::stoi(seperated_fields[2]);
			rand_group = std::stoi(seperated_fields[4]);

			entry->id = std::stoi(seperated_fields[1]);
			entry->amount = std::stoi(seperated_fields[3]);

			if ((int)seperated_fields.size() >= 6) entry->announce = std::stoi(seperated_fields[5]);
			if ((int)seperated_fields.size() >= 7) entry->rare = std::stoi(seperated_fields[6]);

			if ((int)seperated_fields.size() >= 8) {
				std::string weight_field;
				std::istringstream weight_data(seperated_fields[7]);

				while (std::getline(weight_data, weight_field, ':')) {
					entry->rnd_weight.push_back( std::stoi(weight_field) );
				}
			}
			
			// checking group
			{
				// find group id
				auto it = group_db.find(group_id);

				// if there's no group id in container then create new one
				if (it == group_db.end()) {
					struct item_group_db* group = new item_group_db();
					group_db[group_id] = group;
					
					// generate new random group
					for (int k = 0; k < MAX_RAND_GROUP; ++k) {
						group->random.push_back(std::make_shared<item_group_random>());
					}
				}
			}

			// get group
			auto it = group_db[group_id];
			if (!it) throw "Error, unexpected to happen";
			
			// if it's a must item
			if (rand_group == 0) {
				it->must.push_back(entry);

				// skip to next process
				if (prob == 0) continue;
				rand_group = 0;
			}
			else {
				rand_group -= 1;
			}
			 
			random = it->random[rand_group];

			if (!random) throw "Error, Random group is nullptr!";
			
			for (j = 0; j < prob; ++j) {
				random->data.push_back(entry);
			}
		}
	}
}

ItemDB::~ItemDB() {
	for (auto &it : group_db) {
		// clear must entry
		it.second->must.clear();

		// clear random entry
		for (auto &rand : it.second->random) {
			rand->data.clear();
		}

		// clear random data
		it.second->random.clear();
	}

	// clear group data
	group_db.clear();
	item_data.clear();
	part_data.clear();
	club_data.clear();
	ball_data.clear();
	card_data.clear();
	magicbox_data.clear();
}