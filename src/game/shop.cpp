#include "shop.h"
#include "pc.h"
#include "itemdb.h"
#include "../common/packet.h"

ShopSystem* shop = nullptr;

void ShopSystem::pc_entershop(pc* pc) {
	Packet packet;
	packet.write<uint16>(0x20e);
	packet.write_null(8); // ??
	pc->send_packet(&packet);
}

void ShopSystem::pc_buyitem(pc* pc) {
	uint8 buy_type = pc->read<uint8>();
	
	switch (buy_type) {
	case BUY_NORMAL:
		pc_buyitem_normal(pc);
		break;
	case BUY_RENT:
		pc_buyitem_rent(pc);
		break;
	}
}

void ShopSystem::pc_buyitem_normal(pc* pc) {
	uint16 total = pc->read<uint16>();
	uint32 item_amount = 0, pang_amount = 0, cookie_amount = 0;

	buy_data buy[MAX_BUY];

	for (int i = 0; i < total; ++i) {
		pc->read((char*)&buy[i].un1, sizeof buy_data);

		printf("typeid = %d \n amount = %d \n price = %d \n", buy[i].item_typeid, buy[i].amount, buy[i].pang_price);
	}

	for (int i = 0; i < total; ++i) {
		// item exist?
		if (!itemdb->exists(buy[i].item_typeid)) {
			send_msg(pc, BUY_FAIL);
			return;
		}
		// item buyable?
		else if (!itemdb->buyable(buy[i].item_typeid)) {
			send_msg(pc, CANNOT_BUY_ITEM);
			return;
		}

		// sum pang and cookie
		std::pair<char, uint32> price_data = itemdb->get_price(buy[i].item_typeid);
		switch (price_data.first) {
		case TYPE_PANG:
		case TYPE_PANG2:
			buy[i].amount = itemdb->get_amount(buy[i].item_typeid) * buy[i].amount; // get real amount of the item : type pang
			pang_amount += price_data.second * buy[i].amount;
			break;
		case TYPE_COOKIE:
			buy[i].amount = itemdb->get_amount(buy[i].item_typeid); // cookie amount should be static
			cookie_amount += price_data.second;
			break;
		}

		printf("pang total %d, cookie total %d , amount is =%d\n", pang_amount, cookie_amount, itemdb->get_amount(buy[i].item_typeid));

		// validate that pc can have this item
		switch (pc->inventory->checkitem(pc, buy[i].item_typeid, buy[i].amount)) {
		case CHECKITEM_PASS:
			break;
		case CHECKITEM_FAIL:
			send_msg(pc, BUY_FAIL);
			return;
			break;
		case CHECKITEM_OVERLIMIT:
			send_msg(pc, TOO_MUCH_ITEM);
			return;
			break;
		case CHECKITEM_DUPLICATED:
			send_msg(pc, ALREADY_HAVEITEM);
			return;
			break;
		}
	}

	for (int i = 0; i < total; ++i) {
		item item;
		item.type_id = buy[i].item_typeid;
		item.amount = buy[i].amount;
		pc->inventory->additem(pc, &item, false, true);
	}

	send_msg(pc, BUY_SUCCESS, true);
}

void ShopSystem::pc_buyitem_rent(pc* pc) {
	uint16 total = pc->read<uint16>();
}

void ShopSystem::send_msg(pc* pc, uint32 code, bool success) {
	Packet packet;
	packet.write<uint16>(0x68);
	packet.write<uint32>(code);

	if (success) {
		packet.write<uint64>(100000000);
		packet.write<uint64>(100000000);
	}

	pc->send_packet(&packet);
}