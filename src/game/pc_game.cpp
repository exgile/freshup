#include "pc.h"
#include "gameplay.h"
#include "channel.h"
#include "../common/packet.h"

void pk_leave_game(pc* pc) {
	Packet p;
	p.write<uint16>(0x4c);
	p.write<int16>(-1);
	pc->send_packet(&p);
}

void room_error(pc* pc, roomErr err) {
	Packet p;
	p.write<uint16>(0x49);
	p.write<uint8>((uint8) err);
	pc->send_packet(&p);
}

void Channel::pc_create_game(pc* pc)
{
	std::shared_ptr<gamedata> data = std::make_shared<gamedata>();
	pc->read((char*)&data->un1, sizeof gamedata);
	printf("vs = %d | match = %d | hole total = %d | max player %d | type = %d \n", data->vs_time, data->match_time, data->hole_total, data->max_player, data->game_type);

	game* game = nullptr;

	switch (data->game_type) {
	case gtStroke:
	{
		uint32 natural = pc->read<uint32>();
		std::string name = pc->read<std::string>();
		std::string pwd = pc->read<std::string>();
		uint32 artifact = pc->read<uint32>();
		game = new game_stroke(data, room_id->get(), name, pwd);
		game->natural = natural;
	}
	break;
	case gtChatroom:
	{
		uint32 natural = pc->read<uint32>();
		std::string name = pc->read<std::string>();
		std::string pwd = pc->read<std::string>();
		uint32 artifact = pc->read<uint32>();
		game = new game_chatroom(data, room_id->get(), name, pwd);
		game->natural = natural;
	}
	break;
	}

	if (!game) {
		room_error(pc, rFail);
		return;
	}

	game_list.push_back(game);

	game->channel = this;
	game->addmaster(pc);
}

void Channel::pc_join_game(pc* pc) {
	uint16 req_id = pc->read<uint16>();
	std::string req_pwd = pc->read<std::string>();
	game* game = sys_getgame_byid(req_id);

	if ( (game == nullptr) || (!game->valid)) {
		room_error(pc, rNotExist);
		return;
	}

	if ((uint8)game->pc_list.size() >= game->maxplayer) {
		room_error(pc, rFull);
		return;
	}

	// if password not match or not gm
	if ( game->password.compare(req_pwd) != 0 && pc->capability_ != 4) {
		room_error(pc, rPwdErr);
		return;
	}

	game->addpc(pc);
}

void Channel::pc_leave_game(pc* pc) 
{
	game* game = pc->game;

	sys_veriy_game(game);
	
	if (game->pc_remove(pc)) {
		pc->game = nullptr;
		pc->game_id = -1;
		sys_pc_action(pc, lbUpdate);

		game->sys_send_pcleave(pc);

		if ((uint8)game->pc_list.size() > 0) 
			sys_game_action(game, gUpdate);

		game->sys_inspec();
		pk_leave_game(pc);
	}
}

void pc::change_equipment() 
{
	Packet p;
	uint8 action = read<uint8>();
	uint32 value = read<uint32>();

	p.write<uint16>(0x4b);
	p.write<uint32>(0);
	p.write<uint8>(action);
	p.write<uint32>(connection_id_);

	switch (action) {
	case e_char:
		warehouse->write_current_char(&p);
		break;
	}

	if (action == e_char) {
		if (game == nullptr) return;
		game->send(&p);
	}
	else {
		send_packet(&p);
	}
}

void pc_roomaction(pc* pc) {
	if (pc->game == nullptr) 
		return;
	pc->game->pc_action(pc);
}

void game_setting(pc* pc) {
	if (pc->game == nullptr)
		return;

	pc->game->pc_change_game_config(pc);
}