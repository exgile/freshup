#include "gameplay.h"
#include "channel.h"
#include "pc.h"

#include "../common/utils.h"
#include "../common/packet.h"

game::game(std::shared_ptr<gamedata> const& data, uint16 room_id, std::string const& room_name, std::string const& room_pwd)
{
	roomId = room_id;
	vs_time = data->vs_time;
	match_time = data->match_time;
	maxplayer = data->max_player;
	game_type = data->game_type;
	hole_total = data->hole_total;
	map = data->map;
	mode = data->mode;

	name = room_name;
	password = room_pwd;

	genkey();
}

void game::addmaster(pc* pc)
{
	pc_list.push_back(pc);
	pc->game = this;
	pc->game_id = roomId;
	pc->game_role = 8;

	pc->game_position.init();
	pc->posture = 0;
	pc->animate = 0;

	sys_calc_pcslot();
	gameupdate();
	roomdata(pc);
	send_pc_create(pc);

	channel->sys_game_action(this, gCreate);
	channel->sys_pc_action(pc, lbUpdate);
}

void game::addpc(pc* pc) 
{
	pc_list.push_back(pc);
	pc->game = this;
	pc->game_id = roomId;
	pc->game_role = 1;

	pc->game_position.init();
	pc->posture = 0;
	pc->animate = 0;

	sys_calc_pcslot();
	gameupdate();
	roomdata(pc);

	// send pc list
	channel->sys_game_action(this, gUpdate);
	channel->sys_pc_action(pc, lbUpdate);

	send_pc_join(pc);
}

void game::genkey() 
{
	for (int i = 0; i < sizeof gameKey; ++i) {
		gameKey[i] = utils::random_int(1, 255);
	}
}

void game::send(Packet* p) 
{
	std::for_each(pc_list.begin(), pc_list.end(), [p](pc* pc) {
		pc->send_packet(p);
	});
}

bool game::pc_remove(pc* pc) 
{
	auto it = std::find(pc_list.begin(), pc_list.end(), pc);
	if (it == pc_list.end()) { return false; }

	pc_list.erase(it);

	return true;
}

void game::pc_action(pc* pc) {
	sys_verify_pc(pc);

	uint8 action = pc->read<uint8>();

	Packet p;
	p.write<uint16>(0xc4);
	p.write<uint32>(pc->connection_id_);
	p.write<uint8>(action);

	switch (action) {
	case rPCMoveAround:
	{
		float pos = pc->read<float>();
		p.write<float>(pos);
	}
	break;
	case rPCVSAnimate:
	{
		std::string animate = pc->read<std::string>();
		p.write<std::string>(animate);
	}
	break;
	case rPCAppear:
	{
		pc->read((char*)&pc->game_position.x, sizeof Pos3D);
		p.write((char*)&pc->game_position.x, sizeof Pos3D);
	}
	break;
	case rPCMove:
	{
		Pos3D pos;
		pc->read((char*)&pos.x, sizeof Pos3D);
		pc->game_position += pos;
		p.write((char*)&pos.x, sizeof Pos3D);
	}
	break;
	case rPCPosture:
	{
		pc->posture = pc->read<uint32>();
		p.write<uint32>(pc->posture);
	}
	break;
	case rPCAction:
	{
		std::string action = pc->read<std::string>();
		p.write<std::string>(action);
	}
	break;
	case rPCAnimation:
	{
		pc->animate = pc->read<uint32>();
		p.write<uint32>(pc->animate);
	}
	break;
	}

	send(&p);
}

void game::sys_verify_pc(pc* pc) 
{
	auto it = std::find(pc_list.begin(), pc_list.end(), pc);
	if (it == pc_list.end()) throw "PC not found.";
}

void game::sys_inspec()
{
	if (pc_list.size() == 0) {
		valid = false;
		channel->sys_game_action(this, gDestroy);
	}
}

void game::sys_weather(uint8 weather)
{
	Packet p;
	p.write<uint16>(0x9e);
	p.write<uint8>(weather);
	p.write<uint16>(0);

	send(&p);
}

void game::gameupdate() 
{
	Packet p;
	p.write<uint16>(0x4a);
	p.write<int16>(-1);
	p.write<uint8>(game_type);
	p.write<uint8>(map);
	p.write<uint8>(hole_total);
	p.write<uint8>(mode);
	p.write<uint32>(0); // natural mode
	p.write<uint8>(maxplayer);
	p.write<uint8>(30);
	p.write<uint8>(0); // room idle
	p.write<uint32>(vs_time);
	p.write<uint32>(match_time);
	p.write<uint32>(0); // trophy typeid
	p.write<uint8>( password.length() > 0 ? 0 : 1 );
	p.write<std::string>(name);

	send(&p);
}

void game::roomdata(Packet* p) 
{
	p->write_string(name, 40);
	p->write_null(24);
	p->write<uint8>(password.length() > 0 ? 0 : 1);
	p->write<uint8>( started ? 0 : 1);
	p->write<uint8>(0); // orange
	p->write<uint8>(maxplayer);
	p->write<uint8>((uint8)pc_list.size());
	p->write((char*)&gameKey[0], sizeof gameKey);
	p->write<uint8>(0);
	p->write<uint8>(0x1e);
	p->write<uint8>(hole_total);
	p->write<uint8>(game_type);
	p->write<uint16>(roomId);
	p->write<uint8>(mode);
	p->write<uint8>(map);
	p->write<uint32>(vs_time);
	p->write<uint32>(match_time);
	p->write<uint32>(0); // trophy
	p->write<uint8>(0); // idile?
	p->write<uint8>(0); // 1 = gm event , 0 = normal
	p->write_null(0x4a);
	p->write<uint32>(100);
	p->write<uint32>(100);
	p->write<uint32>(owner_uid);
	p->write<uint8>(0xff); // practice?
	p->write<uint32>(0); // artifact
	p->write<uint32>(0); // natural mode
	p->write<uint32>(0); // grandprix 1
	p->write<uint32>(0); // grandprix 2
	p->write<uint32>(0); // grandprix time
	p->write<uint32>(0); // is grandprix?
}

void game::sys_send_pcleave(pc* pc) {
	Packet p;
	p.write<uint16>(0x48);
	p.write<int8>(2);
	p.write<int16>(-1);
	p.write<uint32>(pc->connection_id_);
	send(&p);
}

void game::roomdata(pc* pc) 
{
	Packet p;
	p.write<uint16>(0x49);
	p.write<uint16>(0);
	roomdata(&p);
	pc->send_packet(&p);
}