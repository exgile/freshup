#include "item.h"

#include "../common/packet.h"
#include "../common/typedef.h"

#include "itemdb.h"
#include "pc.h"

void pc_opencardpack(pc* pc) {
	assert(pc);
	uint32 card_typeid = pc->read<uint32>(); // get card typeid

	std::shared_ptr<INV_CARD> card_ret = std::make_shared<INV_CARD>();
	char ret = pc->inventory->delcard(pc, card_typeid, 1, false, 2, &card_ret);

	if (ret != DELITEM_SUCCESS) {
		clif_cardopenfail(pc);
		return;
	}

	std::vector<uint32> pack_data = itemdb->get_cardpack(card_typeid);

	if ((uint8)pack_data.size() <= 0) {
		clif_cardopenfail(pc);
		return;
	}

	Packet packet;
	packet.write<uint16>(0x154);
	packet.write<uint32>(0); // success
	packet.write<uint32>(card_ret->id);
	packet.write<uint32>(card_ret->card_typeid);
	packet.write_null(0x0c);
	packet.write<uint32>(1);
	packet.write_null(0x20);
	packet.write<uint16>(1);
	packet.write<uint8>((uint8)pack_data.size());

	for (int i = 0; i < (uint8)pack_data.size(); ++i) {
		item item;
		item.type_id = pack_data[i];
		item.amount = 1;
		SP_INV_TRANSACTION tran_ret = std::make_shared<INV_TRANSACTION>();
		pc->inventory->addCard(pc, &item, false, false, &tran_ret);

		packet.write<uint32>(tran_ret->item_id);
		packet.write<uint32>(tran_ret->item_typeid);
		packet.write_null(0xc);
		packet.write<uint32>(tran_ret->new_amount);
		packet.write_null(0x20);
		packet.write<uint16>(1);
		packet.write<uint32>(1);
	}

	pc->send_packet(&packet);
}

void clif_cardopenfail(pc* pc) {
	Packet packet;
	packet.write<uint16>(0x154);
	packet.write<uint32>(1);
	pc->send_packet(&packet);
}