#include "inventory.h"
#include "pc.h"
#include "utils.h"

char Inventory::delitem(pc* pc, uint32 item_typeid, uint32 amount, bool transaction, uint8 type_num) {
	uint32 item_type = utils::itemdb_type(item_typeid);
	switch (item_type) {
	case ITEMDB_CHAR:
		break;
	case ITEMDB_USE:
		return deluse(pc, item_typeid, amount, transaction, type_num);
		break;
	case ITEMDB_CARD:
		return delcard(pc, item_typeid, amount, transaction, type_num);
		break;
	}

	return DELITEM_ERROR;
}

char Inventory::deluse(pc* pc, uint32 item_typeid, uint32 amount, bool transaction, uint8 type_num) {
	auto item = std::find_if(item_.begin(), item_.end(), [&item_typeid](std::shared_ptr<INV_ITEM> const& find) {
		return find->item_typeid == item_typeid && find->valid == 1;
	});

	if (item == item_.end()) return DELITEM_ITEM_NOTFOUND;

	if ((*item)->c0 < amount) return DELITEM_AMOUNT_NOTENOUGHT;

	int old_amount = (*item)->c0;
	(*item)->c0 -= amount;
	if ((*item)->c0 <= 0) {
		(*item)->valid = false;
	}

	(*item)->sync = true;

	if (transaction) {
		add_transaction(type_num, (*item), old_amount);
	}

	return DELITEM_SUCCESS;
}

char Inventory::delcard(pc* pc, uint32 item_typeid, uint32 amount, bool transaction, uint8 type_num, std::shared_ptr<INV_CARD>* card_out) {
	auto card = std::find_if(card_.begin(), card_.end(), [&item_typeid](std::shared_ptr<INV_CARD> const& find) {
		return find->card_typeid == item_typeid && find->valid == 1;
	});

	if (card == card_.end()) return DELITEM_ITEM_NOTFOUND;

	if ((*card)->amount < amount) return DELITEM_AMOUNT_NOTENOUGHT;

	int old_amount = (*card)->amount;
	(*card)->amount -= amount;
	if ((*card)->amount <= 0) {
		(*card)->valid = false;
	}

	(*card)->sync = true;

	if (transaction) {
		add_transaction(type_num, (*card), old_amount);
	}

	if (card_out) card_out->reset(new INV_CARD(**card));

	return DELITEM_SUCCESS;
}