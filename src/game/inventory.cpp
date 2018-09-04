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
	stm << "SELECT A.* FROM club_data A "
		<< "INNER JOIN inventory B ON B.id = A.item_id AND B.account_id = ? AND B.valid = 1", use(pc->account_id_), now;
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

/* new warehouse implement */
PC_Warehouse::PC_Warehouse() :
	equipment_(std::make_shared<PC_Equipment>()) {}

void PC_Warehouse::pc_load_data(pc* pc) {
	Poco::Data::Session sess = sdb->get_session();

	/* Load char data from sql */
	{
		Poco::Data::Statement stm(sess);
		stm << "SELECT * FROM char WHERE account_id = ?", use(pc->account_id_), now;
		Poco::Data::RecordSet rs(stm);

		if (rs.rowCount() <= 0) {
			pc->disconnect();
			return;
		}

		bool done = rs.moveFirst();

		while (done) {
			std::shared_ptr<Item> item = std::make_shared<Item>();
			item->id = rs["char_id"];
			item->item_typeid = rs["char_typeid"];
			item->hair_colour = rs["hair_color"];
			item->c0 = rs["c0"];
			item->c1 = rs["c1"];
			item->c2 = rs["c2"];
			item->c3 = rs["c3"];
			item->c4 = rs["c4"];
			item->flag = rs["flag"];
			inventory_.push_back(item);
			done = rs.moveNext();
		}
	}

	/* load char equipment */
	{
		Poco::Data::Statement stm(sess);
		stm << "SELECT * FROM char_equip WHERE account_id = ?", use(pc->account_id_), now;
		Poco::Data::RecordSet rs(stm);

		bool done = rs.moveFirst();

		while (done) {
			std::shared_ptr<Char_Equip> char_eqp = std::make_shared<Char_Equip>();
			char_eqp->char_id = rs["char_id"];
			char_eqp->item_id = rs["item_id"];
			char_eqp->item_typeid = rs["item_typeid"];
			char_eqp->num = rs["num"];
			char_equip_.push_back(char_eqp);
			done = rs.moveNext();
		}
	}

	/* Load item data from sql */
	{
		Poco::Data::Statement stm(sess);
		stm << "SELECT * FROM inventory WHERE account_id = ? AND valid = 1", use(pc->account_id_), now;
		Poco::Data::RecordSet rs(stm);

		bool done = rs.moveFirst();

		while (done) {
			std::shared_ptr<Item> item = std::make_shared<Item>();
			item->id = rs["id"];
			item->item_typeid = rs["typeid"];
			item->c0 = rs["c0"];
			item->c1 = rs["c1"];
			item->c2 = rs["c2"];
			item->c3 = rs["c3"];
			item->c4 = rs["c4"];
			item->flag = rs["flag"];
			item->type = rs["item_type"];
			item->create_date = (Poco::DateTime)rs["reg_date"];
			item->end_date = (Poco::DateTime)rs["end_date"];
			inventory_.push_back(item);
			done = rs.moveNext();
		}
	}

	/* Load ClubSet data from sql */
	{
		Poco::Data::Statement stm(sess);
		stm << "SELECT A.* FROM club_data A "
			<< "INNER JOIN inventory B ON B.id = A.item_id AND B.account_id = ? AND B.valid = 1", use(pc->account_id_), now;

		Poco::Data::RecordSet rs(stm);

		bool done = rs.moveFirst();

		while (done) {
			std::shared_ptr<Club_Data> club = std::make_shared<Club_Data>();
			club->item_id = rs["item_id"];
			club->c0 = rs["c0"];
			club->c1 = rs["c1"];
			club->c2 = rs["c2"];
			club->c3 = rs["c3"];
			club->c4 = rs["c4"];
			club->point = rs["point"];
			club->work_count = rs["work_count"];
			club->cancel_count = rs["cancel_count"];
			club->point_total = rs["point_total"];
			club->pang_total = rs["pang_total"];
			club_data_.push_back(club);
			done = rs.moveNext();
		}
	}

	/* Load card from sql */
	{
		Poco::Data::Statement stm(sess);
		stm << "SELECT * FROM card WHERE account_id = ? AND valid = 1", use(pc->account_id_), now;
		Poco::Data::RecordSet rs(stm);

		bool done = rs.moveFirst();
		while (done) {
			std::shared_ptr<Item> item = std::make_shared<Item>();
			item->id = rs["id"];
			item->item_typeid = rs["typeid"];
			item->c0 = rs["amount"];
			inventory_.push_back(item);
			done = rs.moveNext();
		}
	}

	/* Load pc equipment */
	{
		Poco::Data::Statement stm(sess);
		stm << "SELECT * FROM equipment WHERE account_id = ?", use(pc->account_id_), now;

		Poco::Data::RecordSet rs(stm);

		if (rs.rowCount() > 0) {
			equipment_->caddie_id = rs["caddie_id"];
			equipment_->club_id = rs["club_id"];
			equipment_->char_id = rs["character_id"];
			equipment_->ball_id = rs["ball_typeid"];
			equipment_->item_slot[0] = rs["item_slot_1"];
			equipment_->item_slot[1] = rs["item_slot_2"];
			equipment_->item_slot[2] = rs["item_slot_3"];
			equipment_->item_slot[3] = rs["item_slot_4"];
			equipment_->item_slot[4] = rs["item_slot_5"];
			equipment_->item_slot[5] = rs["item_slot_6"];
			equipment_->item_slot[6] = rs["item_slot_7"];
			equipment_->item_slot[7] = rs["item_slot_8"];
			equipment_->item_slot[8] = rs["item_slot_9"];
			equipment_->item_slot[9] = rs["item_slot_10"];
			equipment_->mascot_id = rs["mascot_id"];
		}
	}
}

int PC_Warehouse::item_count(inventory_type type_name) {
	int count = 0;

	switch (type_name) {
	case IV_CHAR:
		for (auto &item : inventory_) {
			if (utils::itemdb_type(item->item_typeid) == ITEMDB_CHAR) {
				count += 1;
			}
		}
		break;
	case IV_ALLITEM:
		for (auto &item : inventory_) {
			uint8 item_type = utils::itemdb_type(item->item_typeid);
			if (item_type == ITEMDB_PART
				|| item_type == ITEMDB_CLUB
				|| item_type == ITEMDB_BALL
				|| item_type == ITEMDB_USE
				|| item_type == ITEMDB_SKIN
				|| item_type == ITEMDB_AUX)
			{
				count += 1;
			}
		}
		break;
	case IV_CARD:
		for (auto &item : inventory_) {
			if (utils::itemdb_type(item->item_typeid) == ITEMDB_CARD) {
				count += 1;
			}
		}
		break;
	}

	return count;
}

uint16 PC_Warehouse::get_time_left(std::shared_ptr<Item> const& item) {
	if (pc_item_isrent(item->flag)) {
		return static_cast<uint32>((item->end_date.timestamp().epochTime() - item->create_date.timestamp().epochTime()) / (3600));
	}
	return 0;
}

void PC_Warehouse::pc_send_data(pc* pc, inventory_type type_name) {

	Packet p;
	int count = 0;

	switch (type_name) {
	case IV_CHAR:
	/* New Scope */
	{
		count = item_count(IV_CHAR);
		p.write<uint16>(0x70);
		p.write<uint16>(count);
		p.write<uint16>(count);

		for (auto &item : inventory_) {
			if (utils::itemdb_type(item->item_typeid) == ITEMDB_CHAR) {
				p.write<uint32>(item->item_typeid);
				p.write<uint32>(item->id);
				p.write<uint16>(item->hair_colour);
				p.write<uint16>(item->flag);

				/* char equipment */
				for (int i = 1; i <= 24; ++i) {
					auto eqp = std::find_if(char_equip_.begin(), char_equip_.end(), [&i](std::shared_ptr<Char_Equip> const& c_eqp) {
						return c_eqp->num == i;
					});

					if (eqp != char_equip_.end()) {
						p.write<uint32>((*eqp)->item_typeid);
						p.write<uint32>((*eqp)->item_id);
					}
				}

				p.write_null(0xd8);
				p.write<uint32>(0); // left ring
				p.write<uint32>(0); // right ring
				p.write_null(12); // ??
				p.write<uint32>(0); // cutin index
				p.write_null(12); // ??
				p.write<uint8>((uint8)item->c0);
				p.write<uint8>((uint8)item->c1);
				p.write<uint8>((uint8)item->c2);
				p.write<uint8>((uint8)item->c3);
				p.write<uint8>((uint8)item->c4);
				p.write<uint8>(0); // mastery point
				p.write_null(3);
				p.write_null(40); // card data
				p.write<uint32>(0);
				p.write<uint32>(0);
			}
		}
	}
	/* End Scope */
	break;
	case IV_ALLITEM:
	/* New Scope */
	{
		count = item_count(IV_ALLITEM);
		p.write<uint16>(0x73);
		p.write<uint16>(count);
		p.write<uint16>(count);

		for (auto &item : inventory_) {
			uint8 item_type = utils::itemdb_type(item->item_typeid);
			if (item_type == ITEMDB_PART
				|| item_type == ITEMDB_CLUB
				|| item_type == ITEMDB_BALL
				|| item_type == ITEMDB_USE
				|| item_type == ITEMDB_SKIN
				|| item_type == ITEMDB_AUX)
			{
				p.write<uint32>(item->id);
				p.write<uint32>(item->item_typeid);
				p.write<uint32>(get_time_left(item));
				p.write<uint16>(item->c0);
				p.write<uint16>(item->c1);
				p.write<uint16>(item->c2);
				p.write<uint16>(item->c3);
				p.write<uint16>(item->c4);
				p.write_null(1);
				p.write<uint8>(item->flag);

				p.write<uint32>(pc_item_isrent(item->flag) ? (uint32)item->create_date.timestamp().epochTime() : 0);
				p.write<uint32>(0);
				p.write<uint32>(pc_item_isrent(item->flag) ? (uint32)item->end_date.timestamp().epochTime() : 0);
				p.write_null(4);
				p.write<uint8>(2);
				p.write_null(16); // ucc name
				p.write_null(25);
				p.write_null(9); //ucc unique
				p.write<uint8>(0); // ucc status
				p.write<uint16>(0); // copy count
				p.write_null(16); // drawer
				p.write_null(60);

				if (item_type == ITEMDB_CLUB) {
					// club status
					auto club_data = std::find_if(club_data_.begin(), club_data_.end(), [&item](std::shared_ptr<Club_Data> const& cd) {
						return cd->item_id == item->id;
					});

					bool valid = club_data != club_data_.end();
					p.write<uint16>(valid ? (*club_data)->c0 : 0);
					p.write<uint16>(valid ? (*club_data)->c1 : 0);
					p.write<uint16>(valid ? (*club_data)->c2 : 0);
					p.write<uint16>(valid ? (*club_data)->c3 : 0);
					p.write<uint16>(valid ? (*club_data)->c4 : 0);
					p.write<uint32>(valid ? (*club_data)->point : 0);
					p.write<uint32>(valid ? (*club_data)->cancel_count : 0);
					p.write<uint32>(valid ? (*club_data)->work_count : 0);
				}
				else {
					p.write_null(22);
				}

				p.write<uint32>(0);
			}
		}
	}
	/* End Scope */
	break;
	case IV_CARD:
	/* New Scope */
	{
		count = item_count(IV_CARD);
		p.write<uint16>(0x138);
		p.write<uint32>(0);
		p.write<uint16>(count);

		for (auto &item : inventory_) {
			if (utils::itemdb_type(item->item_typeid) == ITEMDB_CARD) {
				p.write<uint32>(item->id);
				p.write<uint32>(item->item_typeid);
				p.write_null(12);
				p.write<uint32>(item->c0);
				p.write_null(0x20);
				p.write<uint16>(1);
			}
		}
	}
	/* End Scope */
	break;
	case IV_EQUIPMENT:
	/* New Scope */
	{
		p.write<uint16>(0x72);
		p.write<uint32>(equipment_->caddie_id);
		p.write<uint32>(equipment_->char_id);
		p.write<uint32>(equipment_->club_id);
		p.write<uint32>(equipment_->ball_id);
		p.write<uint32>(equipment_->item_slot[0]);
		p.write<uint32>(equipment_->item_slot[1]);
		p.write<uint32>(equipment_->item_slot[2]);
		p.write<uint32>(equipment_->item_slot[3]);
		p.write<uint32>(equipment_->item_slot[4]);
		p.write<uint32>(equipment_->item_slot[5]);
		p.write<uint32>(equipment_->item_slot[6]);
		p.write<uint32>(equipment_->item_slot[7]);
		p.write<uint32>(equipment_->item_slot[8]);
		p.write<uint32>(equipment_->item_slot[9]);
		p.write<uint32>(0);
		p.write<uint32>(0);
		p.write<uint32>(0);
		p.write<uint32>(0);
		p.write<uint32>(0);
		p.write<uint32>(0); // title index
		p.write<uint32>(0);
		p.write<uint32>(0);
		p.write<uint32>(0);
		p.write<uint32>(0);
		p.write<uint32>(0);
		p.write<uint32>(0); // title typeid
		p.write<uint32>(equipment_->mascot_id);
		p.write<uint32>(0); // poster1
		p.write<uint32>(0); // poster 2
	}
	/* End Scope */
	break;
	}

	pc->send_packet(&p);
}

char PC_Warehouse::additem(pc* pc, item* item, bool transaction, bool from_shop) {
	assert(pc && item);

	auto item = std::find_if(inventory_.begin(), inventory_.end(), []() {
	});
}