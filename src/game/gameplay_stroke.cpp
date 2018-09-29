#include "gameplay.h"
#include "channel.h"
#include "pc.h"

#include "../common/utils.h"
#include "../common/packet.h"

game_stroke::game_stroke(std::shared_ptr<gamedata> const& data, uint16 room_id, std::string const& room_name, std::string const& room_pwd)
	: game(data, room_id, room_name, room_pwd){}

void game_stroke::send_pc_create(pc* pc) {
	Packet p;
	p.write<uint16>(0x48);
	p.write<uint8>(0);
	p.write<int16>(-1);
	p.write<uint8>((uint8)pc_list.size());
	pc->gamedata(&p, true);
	p.write<uint8>(0);

	send(&p);
}

void game_stroke::send_pc_join(pc* pc) {
	// query all pc
	{
		Packet p;
		p.write<uint16>(0x48);
		p.write<uint8>(0);
		p.write<int16>(-1);
		p.write<uint8>((uint8)pc_list.size());
		for (auto &it : pc_list) {
			it->gamedata(&p, true);
		}
		p.write<uint8>(0);
		pc->send_packet(&p);
	}

	// query this pc to all
	{
		Packet p;
		p.write<uint16>(0x48);
		p.write<uint8>(1);
		p.write<int16>(-1);
		pc->gamedata(&p, true);
		send(&p);
	}
}

void game_stroke::sys_calc_pcslot()
{
	int slot = 1;

	for (auto &it : pc_list) {
		it->game_slot = slot;
		slot += 1;
	}
}