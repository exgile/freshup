#include "inventory.h"
#include "pc.h"
#include "../common/packet.h"
#include "../common/db.h"
#include "itemdb.h"
#include "utils.h"

using namespace Poco::Data::Keywords; 

uint32 Inventory::sys_cal_hour_left(std::shared_ptr<INV_ITEM> const& item) {
	if (sys_is_rent(item->flag)) {
		return static_cast<uint32>( (item->end_date.timestamp().epochTime() - item->reg_date.timestamp().epochTime()) / (3600));
	}
	return 0;
}

bool Inventory::sys_is_rent(uint8 rent_flag){
	if (rent_flag == ITEMDB_PERIOD ||
		rent_flag == ITEMDB_SKIN_PERIOD ||
		rent_flag == ITEMDB_PART_RENT)
	{
		return true;
	}
	return false;
}

bool Inventory::pc_item_exists(uint32 val, enum item_type_name find_type, enum find_by by_) {
	if (find_type == ITEMDB_CHAR) {
		auto char_ = std::find_if(character_.begin(), character_.end(), [&val, &by_](std::shared_ptr<INV_CHAR> const& charf) {
			if (by_ == FIND_BY_ID) {
				return charf->id == val;
			}
			else if (by_ == FIND_BY_TYPEID) {
				return charf->char_typeid == val;
			}
			return false;
		});
		if (char_ == character_.end()) {
			return false;
		}
		return true;
	}
	else if (find_type == ITEMDB_USE) {
		auto nitem = std::find_if(item_.begin(), item_.end(), [&val, &by_](std::shared_ptr<INV_ITEM> const &itemf) {
			if (by_ == FIND_BY_ID) {
				return itemf->id == val && itemf->c0 > 0 && itemf->valid == 1;
			}
			else if (by_ == FIND_BY_TYPEID) {
				return itemf->item_typeid == val && itemf->c0 > 0 && itemf->valid == 1;
			}
			return false;
		});
		if (nitem == item_.end()) {
			return false;
		}
		return true;
	}
	// default item not exists to prevent things wrong
	return false;
}

char Inventory::checkitem(pc* pc, uint32 id, uint32 amount) {
	int item_type = utils::itemdb_type(id);

	if (item_type == ITEMDB_USE) {
		auto find_use = std::find_if(item_.begin(), item_.end(), [&id](std::shared_ptr<INV_ITEM> const& use) {
			return use->item_typeid == id && use->valid == 1;
		});

		if (find_use == item_.end()) {
			return CHECKITEM_PASS;
		}

		if ((*find_use)->c0 >= MAX_AMOUNT_ITEM) {
			return CHECKITEM_OVERLIMIT;
		}

		if ((*find_use)->c0 + amount < MAX_AMOUNT_ITEM) {
			return CHECKITEM_PASS;
		}
	}
	else if (item_type == ITEMDB_CARD) {
		return CHECKITEM_PASS;
	}

	return CHECKITEM_FAIL;
}

SP_INV_TRANSACTION Inventory::add_transaction(uint8 types, std::shared_ptr<INV_CHAR> const& char_, bool toVector) {
	assert(char_);
	std::shared_ptr<INV_TRANSACTION> tran = std::make_shared<INV_TRANSACTION>();
	tran->item_id = char_->id;
	tran->item_typeid = char_->char_typeid;
	tran->old_amount = tran->new_amount = 1;
	if (toVector) transaction_.push_back(tran);
	return tran;
}

SP_INV_TRANSACTION Inventory::add_transaction(uint8 types, std::shared_ptr<INV_ITEM> const& item, uint32 old_amount, bool toVector) {
	assert(item);
	std::shared_ptr<INV_TRANSACTION> tran = std::make_shared<INV_TRANSACTION>();
	tran->item_typeid = item->item_typeid;
	tran->item_id = item->id;
	tran->old_amount = old_amount;
	tran->new_amount = item->c0;
	if (toVector) transaction_.push_back(tran);
	return tran;
}

SP_INV_TRANSACTION Inventory::add_transaction(uint8 types, std::shared_ptr<INV_CARD> const& card, uint32 old_amount, bool toVector) {
	assert(card);
	std::shared_ptr<INV_TRANSACTION> tran = std::make_shared<INV_TRANSACTION>();
	tran->item_typeid = card->card_typeid;
	tran->item_id = card->id;
	tran->old_amount = old_amount;
	tran->new_amount = card->amount;
	if (toVector) transaction_.push_back(tran);
	return tran;
}

void Inventory::load_character(pc* pc) {
	// we have to clear char and char equip first because we will use this function again
	// this is automatically destroy object inside these two vectors
	character_.clear();
	character_equip_.clear();

	{
		Poco::Data::Session sess = sdb->get_session();
		Poco::Data::Statement stm(sess);
		stm << "SELECT * FROM char WHERE account_id = ?", use(pc->account_id_), now;
		Poco::Data::RecordSet rs(stm);

		if (!rs.rowCount()) {
			pc->disconnect();
			return;
		}

		bool done = rs.moveFirst();

		while (done) {
			std::shared_ptr<INV_CHAR> char_(new INV_CHAR());
			char_->id = rs["char_id"];
			char_->char_typeid = rs["char_typeid"];
			char_->hair_color = rs["hair_color"];
			char_->c0 = rs["c0"];
			char_->c1 = rs["c1"];
			char_->c2 = rs["c2"];
			char_->c3 = rs["c3"];
			char_->c4 = rs["c4"];
			char_->flag = rs["flag"];
			character_.push_back(char_);
			done = rs.moveNext();
		}
	}

	{
		Poco::Data::Session sess = sdb->get_session();
		Poco::Data::Statement stm(sess);
		stm << "SELECT * FROM char_equip WHERE account_id = ?", use(pc->account_id_), now;
		Poco::Data::RecordSet rs(stm);
		bool done = rs.moveFirst();

		while (done) {
			std::shared_ptr<INV_CHAR_EQUIP> pce(new INV_CHAR_EQUIP());
			pce->char_id = rs["char_id"];
			pce->item_id = rs["item_id"];
			pce->item_typeid = rs["item_typeid"];
			pce->num = rs["num"];
			character_equip_.push_back(pce);
			done = rs.moveNext();
		}
	}
}

void Inventory::load_item(pc* pc) {
	Poco::Data::Session sess = sdb->get_session();
	Poco::Data::Statement stm(sess);
	stm << "SELECT * FROM inventory WHERE account_id = ?", use(pc->account_id_), now;
	Poco::Data::RecordSet rs(stm);
	bool done = rs.moveFirst();

	while (done) {
		std::shared_ptr<INV_ITEM> item(new INV_ITEM());
		item->id = rs["id"];
		item->item_typeid = rs["typeid"];
		item->c0 = rs["c0"];
		item->c1 = rs["c1"];
		item->c2 = rs["c2"];
		item->c3 = rs["c3"];
		item->c4 = rs["c4"];
		item->flag = rs["flag"];
		item->type = rs["item_type"];
		item->reg_date = (Poco::LocalDateTime)rs["reg_date"];
		item->end_date = (Poco::LocalDateTime)rs["end_date"];
		item->valid = rs["valid"];
		item_.push_back(item);

		done = rs.moveNext();
	}
}

void Inventory::load_club_data(pc* pc) {
	Poco::Data::Session sess = sdb->get_session();
	Poco::Data::Statement stm(sess);
	stm << "SELECT * FROM club_data WHERE account_id = ?", use(pc->account_id_), now;
	Poco::Data::RecordSet rs(stm);

	bool done = rs.moveFirst();

	while (done) {
		std::shared_ptr<INV_CLUB_DATA> club_data(new INV_CLUB_DATA());
		club_data->item_id = rs["item_id"];
		club_data->c0 = rs["c0"];
		club_data->c1 = rs["c1"];
		club_data->c2 = rs["c2"];
		club_data->c3 = rs["c3"];
		club_data->c4 = rs["c4"];
		club_data->point = rs["point"];
		club_data->work_count = rs["work_count"];
		club_data->cancel_count = rs["cancel_count"];
		club_data->point_total = rs["point_total"];
		club_data->pang_total = rs["pang_total"];
		club_.push_back(club_data);
		done = rs.moveNext();
	}
}

void Inventory::load_equipment(pc* pc) {
	Poco::Data::Session sess = sdb->get_session();
	Poco::Data::Statement stm(sess);
	stm << "SELECT * FROM equipment WHERE account_id = ?", use(pc->account_id_), now;

	Poco::Data::RecordSet rs(stm);

	if (rs.rowCount() > 0) {
		equipment.caddie_id = rs["caddie_id"];
		equipment.club_id = rs["club_id"];
		equipment.char_id = rs["character_id"];
		equipment.ball_id = rs["ball_typeid"];
		equipment.item_slot[0] = rs["item_slot_1"];
		equipment.item_slot[1] = rs["item_slot_2"];
		equipment.item_slot[2] = rs["item_slot_3"];
		equipment.item_slot[3] = rs["item_slot_4"];
		equipment.item_slot[4] = rs["item_slot_5"];
		equipment.item_slot[5] = rs["item_slot_6"];
		equipment.item_slot[6] = rs["item_slot_7"];
		equipment.item_slot[7] = rs["item_slot_8"];
		equipment.item_slot[8] = rs["item_slot_9"];
		equipment.item_slot[9] = rs["item_slot_10"];
		equipment.mascot_id = rs["mascot_id"];
	}
}

void Inventory::load_card(pc* pc) {
	Poco::Data::Session sess = sdb->get_session();
	Poco::Data::Statement stm(sess);
	stm << "SELECT * FROM card WHERE account_id = ? and valid = 1", use(pc->account_id_), now;
	Poco::Data::RecordSet rs(stm);
	bool done = rs.moveFirst();

	while (done) {
		std::shared_ptr<INV_CARD> card = std::make_shared<INV_CARD>();
		card->id = rs["id"];
		card->card_typeid = rs["typeid"];
		card->amount = rs["amount"];
		card->sync = false;
		card_.push_back(card);
		done = rs.moveNext();
	}
}

void Inventory::send_card(pc* pc) {
	Packet packet;
	packet.write<uint16>(0x138);
	packet.write<uint32>(0);
	packet.write<uint16>((uint16)card_.size());

	for (auto &card : card_) {
		packet.write<uint32>(card->id);
		packet.write<uint32>(card->card_typeid);
		packet.write_null(12);
		packet.write<uint32>(card->amount);
		packet.write_null(0x20);
		packet.write<uint16>(1);
	}

	pc->send_packet(&packet);
}

void Inventory::send_char(pc* pc) {
	Packet packet;
	packet.write<uint16>(0x70);
	packet.write<uint16>((uint16)character_.size());
	packet.write<uint16>((uint16)character_.size());

	for (auto &it : character_) {
		packet.write<uint32>(it->char_typeid);
		packet.write<uint32>(it->id);
		packet.write<uint16>(it->hair_color);
		packet.write<uint16>(it->flag);

		for (int i = 1; i <= 24; ++i) {
			auto ite = std::find_if(character_equip_.begin(), character_equip_.end(), [i](std::shared_ptr<INV_CHAR_EQUIP> m) { return m->num = i; });
			packet.write<uint32>((*ite)->item_typeid);
			packet.write<uint32>((*ite)->item_id);
		}

		packet.write_null(0xd8);
		packet.write<uint32>(0); // left ring
		packet.write<uint32>(0); // right ring
		packet.write_null(12); // ??
		packet.write<uint32>(0); // cutin index
		packet.write_null(12); // ??
		packet.write<uint8>(it->c0);
		packet.write<uint8>(it->c1);
		packet.write<uint8>(it->c2);
		packet.write<uint8>(it->c3);
		packet.write<uint8>(it->c4);
		packet.write<uint8>(0); // mastery point
		packet.write_null(3);
		packet.write_null(40); // card data
		packet.write<uint32>(0);
		packet.write<uint32>(0);
	}

	pc->send_packet(&packet);
}

void Inventory::send_equipment(pc* pc) {
	Packet packet;
	packet.write<uint16>(0x72);
	packet.write<uint32>(equipment.caddie_id);
	packet.write<uint32>(equipment.char_id);
	packet.write<uint32>(equipment.club_id);
	packet.write<uint32>(equipment.ball_id);
	packet.write<uint32>(equipment.item_slot[0]);
	packet.write<uint32>(equipment.item_slot[1]);
	packet.write<uint32>(equipment.item_slot[2]);
	packet.write<uint32>(equipment.item_slot[3]);
	packet.write<uint32>(equipment.item_slot[4]);
	packet.write<uint32>(equipment.item_slot[5]);
	packet.write<uint32>(equipment.item_slot[6]);
	packet.write<uint32>(equipment.item_slot[7]);
	packet.write<uint32>(equipment.item_slot[8]);
	packet.write<uint32>(equipment.item_slot[9]);
	packet.write<uint32>(0);
	packet.write<uint32>(0);
	packet.write<uint32>(0);
	packet.write<uint32>(0);
	packet.write<uint32>(0);
	packet.write<uint32>(0); // title index
	packet.write<uint32>(0);
	packet.write<uint32>(0);
	packet.write<uint32>(0);
	packet.write<uint32>(0);
	packet.write<uint32>(0);
	packet.write<uint32>(0); // title typeid
	packet.write<uint32>(equipment.mascot_id);
	packet.write<uint32>(0); // poster1
	packet.write<uint32>(0); // poster 2
	pc->send_packet(&packet);
}

void Inventory::send_item(pc* pc) {
	Packet packet;
	packet.write<uint16>(0x73);
	packet.write<uint16>((uint16)item_.size());
	packet.write<uint16>((uint16)item_.size());

	for (auto &item : item_) {
		packet.write<uint32>(item->id);
		packet.write<uint32>(item->item_typeid);
		packet.write<uint32>(sys_cal_hour_left(item));
		packet.write<uint16>(item->c0);
		packet.write<uint16>(item->c1);
		packet.write<uint16>(item->c2);
		packet.write<uint16>(item->c3);
		packet.write<uint16>(item->c4);
		packet.write_null(1);
		packet.write<uint8>(item->flag);
		packet.write<uint32>(sys_is_rent(item->flag) ? (uint32)item->reg_date.timestamp().epochTime() : 0);
		packet.write<uint32>(0);
		packet.write<uint32>(sys_is_rent(item->flag) ? (uint32)item->end_date.timestamp().epochTime() : 0);
		packet.write_null(4);
		packet.write<uint8>(2);
		packet.write_null(16); // ucc name
		packet.write_null(25); 
		packet.write_null(9); //ucc unique
		packet.write<uint8>(0); // ucc status
		packet.write<uint16>(0); // copy count
		packet.write_null(16); // drawer
		packet.write_null(60);

		if (utils::itemdb_type(item->item_typeid) == ITEMDB_CLUB) {
			auto club = std::find_if(club_.begin(), club_.end(), [&item](std::shared_ptr<INV_CLUB_DATA> const& cl) {
				return cl->item_id == item->id;
			});

			if (club != club_.end()) {
				packet.write<uint16>((*club)->c0);
				packet.write<uint16>((*club)->c1);
				packet.write<uint16>((*club)->c2);
				packet.write<uint16>((*club)->c3);
				packet.write<uint16>((*club)->c4);
				packet.write<uint32>((*club)->point);
				packet.write<uint32>((*club)->cancel_count);
				packet.write<uint32>((*club)->work_count);
			}
			else {
				packet.write_null(22);
			}
		}
		else {
			packet.write_null(22);
		}

		packet.write<uint32>(0);
	};

	pc->send_packet(&packet);
}