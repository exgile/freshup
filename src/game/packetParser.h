#pragma once

struct pc;

void parsePakcket_pc_login(pc* pc);
void parsePakcket_select_channel(pc *pc);
void parsePakcket_chat(pc *pc);
void parsePakcket_create_game(pc *pc);
void parsePakcket_join_game(pc *pc);
void parsePakcket_start_game(pc *pc);
void parsePakcket_game_setting(pc *pc);
void parsePakcket_leave_game(pc *pc);
void parsePakcket_sync_room_action(pc *pc);
void parsePakcket_change_equipment(pc *pc);
void parsePakcket_enter_lobby(pc *pc);
void parsePakcket_leave_lobby(pc *pc);
void parsePakcket_gm_command(pc *pc);
void parsePakcket_enter_shop(pc *pc);
void parsePakcket_buy_item(pc *pc);
void parsePakcket_open_cardpack(pc *pc);
void parsePakcket_save_data(pc *pc);

void parsePakcket_load_mail(pc *pc);
void parsePakcket_read_mail(pc *pc);

void parsePakcket_bongdari_window(pc *pc);
void parsePakcket_bongdari_normal(pc *pc);
void parsePakcket_bongdari_big(pc *pc);

void parsePakcket_scratch_window(pc *pc);
void parsePakcket_scratch_play(pc *pc);

void parsePakcket_req_server_data(pc *pc);
void parsePakcket_req_enterlobby_while_inlobby(pc *pc);
void parsePakcket_loadmap_success(pc *pc);
void parsePakcket_game_hole_sync(pc *pc);
void parsePakcket_game_shotdata_sync(pc *pc);