#include "gameplay.h"
#include "channel.h"
#include "pc.h"

#include "../common/utils.h"
#include "../common/packet.h"

game_chatroom::game_chatroom(std::shared_ptr<gamedata> const& data, uint16 room_id, std::string const& room_name, std::string const& room_pwd)
	: game(data, room_id, room_name, room_pwd){}

void game_chatroom::send_pc_created(pc* pc) {
	Packet p;
	p.write<uint16>(0x48);
	p.write<uint8>(0);
	p.write<int16>(-1);
	p.write<uint8>((uint8)pc_list.size());
	pc->gamedata(&p, true);

	send(&p);
}