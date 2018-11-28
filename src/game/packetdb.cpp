#include "pc.h"
#include "packetdb.h"
#include "packetParser.h"

std::unordered_map<int, void(*)(pc* pc)> packetdb;

void packetdb_init() {
	packetdb[pc_login] = &parsePakcket_pc_login;
	packetdb[pc_select_channel] = &parsePakcket_select_channel;
	packetdb[pc_send_message] = &parsePakcket_chat;
	packetdb[pc_create_game_] = &parsePakcket_create_game;
	packetdb[pc_join_game] = &parsePakcket_join_game;
	packetdb[pc_game_config] = &parsePakcket_game_setting;
	packetdb[pc_leave_room] = &parsePakcket_leave_game;
	packetdb[pc_room_action] = &parsePakcket_sync_room_action;
	packetdb[pc_change_equipment] = &parsePakcket_change_equipment;
	packetdb[pc_enter_lobby_] = &parsePakcket_enter_lobby;
	packetdb[pc_leave_lobby_] = &parsePakcket_leave_lobby;
	packetdb[pc_gm_command] = &parsePakcket_gm_command;
	packetdb[pc_enter_shop] = &parsePakcket_enter_shop;
	packetdb[pc_buyitem] = &parsePakcket_buy_item;
	packetdb[pc_open_cardpack] = &parsePakcket_open_cardpack;
	packetdb[pc_loadmail] = &parsePakcket_load_mail;
	packetdb[pc_readmail] = &parsePakcket_read_mail;
	packetdb[pc_req_bongdari_window] = &parsePakcket_bongdari_window;
	packetdb[pc_req_bongdari_normal] = &parsePakcket_bongdari_normal;
	packetdb[pc_req_bongdari_big] = &parsePakcket_bongdari_big;
	packetdb[pc_req_scratch_window] = &parsePakcket_scratch_window;
	packetdb[pc_req_scratch_play] = &parsePakcket_scratch_play;
	packetdb[pc_save_data] = &parsePakcket_save_data;
	packetdb[pc_req_serverdata] = &parsePakcket_req_server_data;
	packetdb[pc_req_enterlobby_in] = &parsePakcket_req_enterlobby_while_inlobby;
	packetdb[pc_start_game] = &parsePakcket_start_game;
	packetdb[pc_loadmap_success] = &parsePakcket_loadmap_success;
	packetdb[pc_game_hole_sync] = &parsePakcket_game_hole_sync;
	packetdb[pc_shotdata_sync] = &parsePakcket_game_shotdata_sync;
}