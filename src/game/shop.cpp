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
	uint16 buy_amount = pc->read<uint16>();
	uint32 item_amount = 0, pang_total = 0, cookie_total = 0;

	buy_data item_buy[MAX_ITEM_BUY];

	for (int i = 0; i < buy_amount; ++i) {
		pc->read((char*)&item_buy[i].un1, sizeof buy_data);
	}

	for (int i = 0; i < buy_amount; ++i) {
		// item exist?
		if (!itemdb->exists(item_buy[i].item_typeid)) {
			send_msg(pc, BUY_FAIL);
			return;
		}
		// item buyable?
		else if (!itemdb->buyable(item_buy[i].item_typeid)) {
			send_msg(pc, CANNOT_BUY_ITEM);
			return;
		}

		// sum pang and cookie
		std::pair<char, uint32> price_data = itemdb->get_price(item_buy[i].item_typeid);

		if (price_data.first == TYPE_PANG || price_data.first == TYPE_PANG2) {
			item_buy[i].amount = itemdb->get_amount(item_buy[i].item_typeid) * item_buy[i].amount; // get real amount of the item : type pang
			pang_total += price_data.second * item_buy[i].amount;
		}
		else if (price_data.first == TYPE_COOKIE) {
			item_buy[i].amount = itemdb->get_amount(item_buy[i].item_typeid); // Item Cookie Amount should be static
			cookie_total += price_data.second;
		}
		else {
			throw "Price type is invalid.";
		}

		printf("pang total %d, cookie total %d , amount is =%d\n", pang_total, cookie_total, itemdb->get_amount(item_buy[i].item_typeid));

		// validate that pc can have this item
		/*switch (pc->inventory->checkitem(pc, buy[i].item_typeid, buy[i].amount)) {
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
		}*/
	}
	
	/*for (int i = 0; i < buy_amount; ++i) {
		item item;
		item.type_id = item_buy[i].item_typeid;
		item.amount = buy[i].amount;
		pc->inventory->additem(pc, &item, false, true);
	}*/

	send_msg(pc, BUY_SUCCESS, true);
}

void ShopSystem::pc_buyitem_rent(pc* pc) {
	uint16 total = pc->read<uint16>();
}

void ShopSystem::send_msg(pc* pc, uint32 code, bool success) {
	Packet p;
	p.write<uint16>(0x68);
	p.write<uint32>(code);

	if (success) {
		p.write<uint64>(100000000);
		p.write<uint64>(100000000);
	}

	pc->send_packet(&p);
}