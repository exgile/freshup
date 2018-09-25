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

	gameupdate();
	roomdata(pc);
	send_pc_created(pc);
}

void game::genkey() {
	for (int i = 0; i < sizeof gameKey; ++i) {
		gameKey[i] = utils::random_int(1, 255);
	}
}

void game::send(Packet* p) {
	std::for_each(pc_list.begin(), pc_list.end(), [p](pc* pc) {
		pc->send_packet(p);
	});
}

void game::gameupdate() {
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

void game::roomdata(pc* pc) {
	Packet p;
	p.write<uint16>(0x49);
	p.write<uint16>(0);
	p.write_string(name, 40);
	p.write_null(24);
	p.write<uint8>(password.length() > 0 ? 0 : 1);
	p.write<uint8>( started ? 0 : 1);
	p.write<uint8>(0); // orange
	p.write<uint8>(maxplayer);
	p.write<uint8>((uint8)pc_list.size());
	p.write((char*)&gameKey[0], sizeof gameKey);
	p.write<uint8>(0);
	p.write<uint8>(0x1e);
	p.write<uint8>(hole_total);
	p.write<uint8>(game_type);
	p.write<uint16>(roomId);
	p.write<uint8>(mode);
	p.write<uint8>(map);
	p.write<uint32>(vs_time);
	p.write<uint32>(match_time);
	p.write<uint32>(0); // trophy
	p.write<uint8>(0); // idile?
	p.write<uint8>(0); // 1 = gm event , 0 = normal
	p.write_null(0x4a);
	p.write<uint32>(100);
	p.write<uint32>(100);
	p.write<uint32>(owner_uid);
	p.write<uint8>(0xff); // practice?
	p.write<uint32>(0); // artifact
	p.write<uint32>(0); // natural mode
	p.write<uint32>(0); // grandprix 1
	p.write<uint32>(0); // grandprix 2
	p.write<uint32>(0); // grandprix time
	p.write<uint32>(0); // is grandprix?
	pc->send_packet(&p);
}