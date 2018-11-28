#include "pc.h"
#include "account.h"
#include "gameplay.h"
#include "channel.h"
#include "shop.h"
#include "itemdb.h"
#include "mail.h"

#include "../common/utils.h"
#include "../common/packet.h"

void parsePakcket_pc_login(pc* pc) {
	pc_req_login(pc);
}

void parsePakcket_select_channel(pc* pc) {
	chm->pc_req_enter_channel(pc);
}

void parsePakcket_chat(pc *pc) {
	pc->channel_->pc_req_chat(pc);
}

void parsePakcket_create_game(pc *pc) {
	pc->channel_->pc_req_create_game(pc);
}

void parsePakcket_join_game(pc *pc) {
	pc->channel_->pc_req_join_game(pc);
}

void parsePakcket_leave_game(pc *pc) {
	pc->game->pc_req_leave_game(pc);
}

void parsePakcket_game_setting(pc *pc) {
	if (pc->game == nullptr)
		return;

	pc->game->pc_change_game_config(pc);
}

void parsePakcket_sync_room_action(pc *pc) {
	if (pc->game == nullptr)
		return;
	pc->game->pc_action(pc);
}

void parsePakcket_change_equipment(pc *pc) {
	pc->change_equipment();
}

void parsePakcket_enter_lobby(pc *pc) {
	pc->channel_->pc_enter_lobby(pc);
}

void parsePakcket_leave_lobby(pc *pc) {
	pc->channel_->pc_leave_lobby(pc);
}

void parsePakcket_gm_command(pc *pc) {
	pc->channel_->sys_gm_command(pc);
}

void parsePakcket_enter_shop(pc *pc) {
	pc_req_entershop(pc);
}

void parsePakcket_buy_item(pc *pc) {
	pc_req_buyitem(pc);
}

void parsePakcket_open_cardpack(pc *pc) {
	itemdb->pc_use_cardpack(pc);
}

void parsePakcket_load_mail(pc *pc) {
	pc_req_loadmail(pc);
}

void parsePakcket_read_mail(pc *pc) {
	pc_req_readmail(pc);
}

void parsePakcket_bongdari_window(pc *pc) {
	Packet p;
	WTHEAD(&p, 0x10b);
	WTI32(&p, -1);
	WTI32(&p, -1);
	WTIU32(&p, 0);
	pc->send(&p);
}

void parsePakcket_bongdari_normal(pc *pc) {
	itemdb->pc_req_bongdari_normal(pc);
}

void parsePakcket_bongdari_big(pc *pc) {
	itemdb->pc_req_bongdari_big(pc);
}

void parsePakcket_scratch_window(pc *pc) {
	Packet p;
	WTHEAD(&p, 0x1EB);
	WTZERO(&p, 5);
	pc->send(&p);
}

void parsePakcket_scratch_play(pc *pc) {
	itemdb->pc_req_scratch_play(pc);
}

void parsePakcket_save_data(pc *pc) {
	pc_req_savedata(pc);
}

void parsePakcket_req_server_data(pc *pc) {
	chm->getserver_data(pc);
}

void parsePakcket_req_enterlobby_while_inlobby(pc *pc) {
	chm->pc_req_enter_channel(pc);
	pc->channel_->pc_enter_lobby(pc);
}

void parsePakcket_start_game(pc *pc) {
	if (pc->game != nullptr) {
		pc->game->startgame();
	}
}

void parsePakcket_loadmap_success(pc *pc) {
	if (pc->game != nullptr) {
		pc->game->pc_loadmap_success(pc);
	}

}

void parsePakcket_game_hole_sync(pc *pc) {
	if (pc->game != nullptr) {
		pc->game->pc_req_holesync(pc);
	}
}

void parsePakcket_game_shotdata_sync(pc *pc) {
	if (pc->game != nullptr) {
		pc->game->pc_req_sync_shotdata(pc);
	}
}