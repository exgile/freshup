#include "gameplay.h"
#include "channel.h"
#include "pc.h"
#include "static.h"

#include "../common/utils.h"
#include "../common/packet.h"

game_practice::game_practice(std::shared_ptr<gamedata> const& data, holepractice& practicedata, uint16 room_id) : 
	game(data, room_id, practicedata.name, practicedata.pwd) 
{
	natural = practicedata.natural;
	practicetype = tPractice;
	game_type = 4;
}

game_practice::game_practice(std::shared_ptr<gamedata> const& data, holerepeat& repeatdata, uint16 room_id) :
	game(data, room_id, repeatdata.name, repeatdata.pwd)
{
	natural = repeatdata.natural;
	practicetype = tRepeat;
	game_type = 4;
	practice_holeposition = repeatdata.holepos;
	practice_lockhole = repeatdata.lockhole;
}

void game_practice::send_pc_create(pc *pc) {
	Packet p;
	WTHEAD(&p, 0x48);
	WTIU08(&p, 0);
	WTI16(&p, -1);
	WTIU08(&p, (uint8)pc_list.size());
	pc->gamedata(&p, false); // get game data
	WTIU08(&p, 0);
	send(&p);
}

void game_practice::send_pc_join(pc *pc) {
	// This is not need for practice game mode.
}

void game_practice::pc_req_gamedata(pc * pc)
{
	Packet p;
	WTHEAD(&p, 0x76);
	WTIU08(&p, 4); // Match Type
	WTIU32(&p, 1);
	WTDATETIME(&p);
	pc->send(&p);

	WRESET(&p);

	WTHEAD(&p, 0x52);
	WTIU08(&p, map);
	WTIU08(&p, 4);
	WTIU08(&p, mode);
	WTIU08(&p, hole_total);
	WTIU32(&p, 0); // Trophy TypeID
	WTIU32(&p, 0); // VS Time
	WTIU32(&p, match_time);

	int kk = 0;
	// Hole Data
	for (auto const &hole : holes) {
		WTIU32(&p, rnd());
		WTIU08(&p, hole.position);
		WTIU08(&p, hole.map);
		WTIU08(&p, ++kk);
	}

	//WTPOINTER(&p, &coindata[0], sizeof(coindata));
	WTZERO(&p, 22);
	pc->send(&p);
}

void game_practice::gameupdate()
{
	Packet p;
	WTHEAD(&p, 0x4A);
	WTI16(&p, -1);
	WTIU08(&p, game_type);
	WTIU08(&p, map);
	WTIU08(&p, hole_total);
	WTIU08(&p, mode);

	if (practicetype == tRepeat) {
		WTIU08(&p, practice_holeposition);
		WTIU32(&p, practice_lockhole);
	}
	
	WTIU32(&p, natural); // NATURAL MODE
	WTIU08(&p, maxplayer);
	WTIU08(&p, 30); // ??
	WTIU08(&p, 0); // ROOM IDLE?
	WTIU32(&p, vs_time);
	WTIU32(&p, match_time);
	WTIU32(&p, 0); // TROPHY TYPEID
	WTIU08(&p, (uint8)password.length() > 0 ? 0 : 1);
	WTCSTR(&p, name);
	send(&p);
}

void game_practice::startgame() {
	game::startgame();
	game::hole_init();
}

void game_practice::pc_loadmap_success(pc * pc)
{
	Packet p;
	WTHEAD(&p, 0x53);
	WTIU32(&p, pc->connection_id_);
	pc->send(&p);
}

void game_practice::pc_req_leave_game(pc * pc)
{
	if (pc_remove(pc)) {
		pc->game = nullptr;
		pc->game_id = -1;

		sys_send_pcleave(pc); // send to all users in the room

		if ((uint8)pc_list.size() == 0) {
			valid = false;
		}

		channel->send_pc_leave_game(pc);
	}
}
