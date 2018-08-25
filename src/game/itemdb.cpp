#include "itemdb.h"
#include "utils.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <fstream>

ItemDB* itemdb = nullptr;

ItemDB::ItemDB() {
	readdb_normal();
	readdb_part();
	readdb_club();
	readdb_ball();
	readdb_card();
	readdb_magicbox();
}

void ItemDB::readdb_normal() {
	std::fstream f;
	int count;

	f.open("db/Item.iff", std::fstream::in | std::fstream::binary);
	f.read((char*)&count, sizeof(count));
	f.seekg(4, std::ios::cur);
	
	for (int i = 1; i <= count; ++i) {
		std::shared_ptr<itemdb_normal> it(new itemdb_normal());
		f.read((char*)&it->base.enable, sizeof(itemdb_normal));
		item_.push_back(it);
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
		std::shared_ptr<itemdb_part> it(new itemdb_part());
		f.read((char*)&it->base.enable, sizeof(itemdb_part));
		part_.push_back(it);
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
		std::shared_ptr<itemdb_club> it(new itemdb_club());
		f.read((char*)&it->base.enable, sizeof(itemdb_club));
		club_.push_back(it);
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
		std::shared_ptr<itemdb_ball> it(new itemdb_ball());
		f.read((char*)&it->base.enable, sizeof(itemdb_ball));
		ball_.push_back(it);
	}

	f.close();
}

void ItemDB::readdb_card() {
	std::fstream f;
	int count;

	f.open("db/Card.iff", std::fstream::in | std::fstream::binary);
	f.read((char*)&count, sizeof(count));
	f.seekg(4, std::ios::cur);

	for (int i = 1; i <= count; ++i) {
		std::shared_ptr<itemdb_card> it(new itemdb_card());
		f.read((char*)&it->base.enable, sizeof(itemdb_card));
		if (strlen(it->base.name) <= 0) continue;
		printf("%d, %s\n" , it->base.item_typeid, it->base.name);
		card_.push_back(it);
	}

	f.close();
}

void ItemDB::readdb_magicbox() {
	std::fstream f;
	uint16 count;

	f.open("db/CadieMagicBox.iff", std::fstream::in | std::fstream::binary);
	f.read((char*)&count, sizeof(count));
	f.seekg(6, std::ios::cur);

	for (int i = 1; i <= count; ++i) {
		std::shared_ptr<magicbox> it(new magicbox());
		f.read((char*)&it->id, sizeof(magicbox));
		magicbox_.push_back(it);
	}

	f.close();
}

bool ItemDB::exists(uint32 id) {
	uint8 item_type = utils::itemdb_type(id);

	if (item_type == ITEMDB_USE) {
		auto find_item = std::find_if(item_.begin(), item_.end(), [&id](std::shared_ptr<itemdb_normal> const& item) {
			return item->base.item_typeid == id;
		});
		if (find_item != item_.end()) {
			return true;
		}
	}
	else if (item_type == ITEMDB_PART) {
		auto find_part = std::find_if(part_.begin(), part_.end(), [&id](std::shared_ptr<itemdb_part> const& part) {
			return part->base.item_typeid == id;
		});
		if (find_part != part_.end()) {
			return true;
		}
	}
	else if (item_type == ITEMDB_CLUB) {
		auto find_club = std::find_if(club_.begin(), club_.end(), [&id](std::shared_ptr<itemdb_club> const& club) {
			return club->base.item_typeid == id;
		});
		if (find_club != club_.end()) {
			return true;
		}
	}
	else if (item_type == ITEMDB_BALL) {
		auto find_ball = std::find_if(ball_.begin(), ball_.end(), [&id](std::shared_ptr<itemdb_ball> const& ball) {
			return ball->base.item_typeid == id;
		});
		if (find_ball != ball_.end()) {
			return true;
		}
	}
	else if (item_type == ITEMDB_CARD) {
		auto find_card = std::find_if(card_.begin(), card_.end(), [&id](std::shared_ptr<itemdb_card> const& card) {
			return card->base.item_typeid == id;
		});
		if (find_card != card_.end()) {
			return true;
		}
	}
	return false;
}

uint32 ItemDB::get_amount(uint32 id) {
	uint8 item_type = utils::itemdb_type(id);

	if (item_type == ITEMDB_USE) {
		auto find_item = std::find_if(item_.begin(), item_.end(), [&id](std::shared_ptr<itemdb_normal> const& item) {
			return item->base.item_typeid == id;
		});
		if (find_item != item_.end()) {
			return (*find_item)->status[0] == 0 ? 1 : (*find_item)->status[0];
		}
	}

	return 1;
}

std::pair<char, uint32> ItemDB::get_price(uint32 id) {
	uint8 item_type = utils::itemdb_type(id);

	if (item_type == ITEMDB_USE) {
		auto find_item = std::find_if(item_.begin(), item_.end(), [&id](std::shared_ptr<itemdb_normal> const& item) {
			return item->base.item_typeid == id;
		});
		if (find_item != item_.end()) {
			return std::make_pair((*find_item)->base.price_type, (*find_item)->base.true_price > 0 ? (*find_item)->base.true_price : (*find_item)->base.price);
		}
	}
	else if (item_type == ITEMDB_CARD) {
		auto find_card = std::find_if(card_.begin(), card_.end(), [&id](std::shared_ptr<itemdb_card> const& card) {
			return card->base.item_typeid == id;
		});
		if (find_card != card_.end()) {
			return std::make_pair((*find_card)->base.price_type, (*find_card)->base.true_price > 0 ? (*find_card)->base.true_price : (*find_card)->base.price);
		}
	}

	return std::make_pair(TYPE_INVALID, 0);
}

bool ItemDB::buyable(uint32 id) {
	uint8 item_type = utils::itemdb_type(id);

	if (item_type == ITEMDB_USE) {
		auto find_item = std::find_if(item_.begin(), item_.end(), [&id](std::shared_ptr<itemdb_normal> const& item) {
			return item->base.item_typeid == id;
		});
		if (find_item != item_.end()) {
			return (*find_item)->base.flag & 1;
		}
	}
	else if (item_type == ITEMDB_CARD) {
		auto find_card = std::find_if(card_.begin(), card_.end(), [&id](std::shared_ptr<itemdb_card> const& card) {
			return card->base.item_typeid == id;
		});
		if (find_card != card_.end()) {
			return (*find_card)->base.flag & 1;
		}
	}

	return false;
}

std::tuple<bool, uint32, uint32> ItemDB::get_cardpackdata(uint32 id) {
	switch (id) {
	case CARD_PACK1:
		return std::make_tuple(true, 1, 3);
		break;
	case CARD_PACK2:
		return std::make_tuple(true, 2, 3);
		break;
	case CARD_PACK3:
		return std::make_tuple(true, 3, 3);
		break;
	case CARD_PACK4:
		return std::make_tuple(true, 4, 3);
		break;
	case CARD_FRESHUP:
		return std::make_tuple(true, 0, 3);
	}

	return std::make_tuple(false, 0, 0);
}

std::vector<uint32> ItemDB::get_cardpack(uint32 id) {
	std::vector<uint32> list;

	/* copy card to temp vector */
	std::vector<std::shared_ptr<itemdb_card>> temp(card_);
	/* shuffle cards from vector */
	std::srand((unsigned int)time(NULL));
	std::random_shuffle(temp.begin(), temp.end());

	/* get card pack data */
	std::tuple<bool, uint32, uint32> data = get_cardpackdata(id);

	if (std::get<0>(data) == false) return list;

	/* card pack no.1 */
	switch (id) {
	case CARD_PACK1:
	case CARD_PACK2:
	case CARD_PACK3:
	case CARD_PACK4:
		while (true) {
			if ((uint8)list.size() >= std::get<2>(data)) {
				break;
			}

			for (auto &it : temp) {
				if (it->base.item_typeid == 2097152001 || it->base.item_typeid == 2097152002 || it->base.item_typeid == 2097152003) continue;
				if (std::find(list.begin(), list.end(), it->base.item_typeid) != list.end()) continue;
				if (it->in_vol == std::get<1>(data)) {
					int rnd = utils::random_int(1, CARD_MAX_PROB);
					if (rnd <= CARD_PROB[it->type]) {
						list.push_back(it->base.item_typeid);
						break;
					}
				}
			}
		}
		break;
	case CARD_FRESHUP:
		while (true) {
			if ((uint8)list.size() >= std::get<2>(data)) {
				break;
			}

			for (auto &it : temp) {
				if (it->base.item_typeid == 2097152001 || it->base.item_typeid == 2097152002 || it->base.item_typeid == 2097152003) continue;
				if (std::find(list.begin(), list.end(), it->base.item_typeid) != list.end()) continue;
				if (it->type >= 1 && (it->in_vol == 1 || it->in_vol == 2 || it->in_vol == 3 || it->in_vol == 4 )) {
					int rnd = utils::random_int(1, CARD_MAX_PROB);
					if (rnd <= CARD_PROB[it->type - 1]) {
						list.push_back(it->base.item_typeid);
						break;
					}
				}
			}
		}

		break;
	}


	return list;
}