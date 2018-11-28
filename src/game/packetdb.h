#pragma once
#include <unordered_map>

struct pc;

extern std::unordered_map<int, void(*)(pc* pc)> packetdb;
void packetdb_init();

enum packet {
	pc_login = 2,
	pc_send_message = 3,
	pc_select_channel = 4,
	pc_create_game_ = 8,
	pc_join_game = 9,
	pc_game_config = 10,
	pc_change_equipment = 12,
	pc_start_game = 14,
	pc_leave_room = 15,
	pc_loadmap_success = 17,
	pc_game_hole_sync = 26,
	pc_shotdata_sync = 0x1b,
	pc_save_data = 32,
	pc_req_serverdata = 0x43,
	pc_room_action = 99,
	pc_enter_lobby_ = 129,
	pc_leave_lobby_ = 130,
	pc_req_enterlobby_in = 0x83,
	pc_gm_command = 143,
	pc_open_cardpack = 202,

	pc_buyitem = 29,
	pc_enter_shop = 320,

	/* Mail System */
	pc_loadmail = 0x143,
	pc_readmail = 0x144,

	/* Papel System*/
	pc_req_bongdari_window = 0x98,
	pc_req_bongdari_normal = 0x14b,
	pc_req_bongdari_big = 0x186,

	/* Scratch Card */
	pc_req_scratch_window = 0x12a,
	pc_req_scratch_play = 0x70
};