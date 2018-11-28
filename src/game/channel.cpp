#include "channel.h"
#include "pc.h"

#include "../common/packet.h"
#include "../common/timer.h"
#include "../common/db.h"
#include "../common/timer.h"

#include "spdlog/sinks/stdout_color_sinks.h"

ChannelManager* chm = nullptr;

ChannelManager::ChannelManager() 
{
	for (uint8 i = 1; i <= 4; ++i) 
	{
		Channel* ch(new Channel());
		ch->id = i;
		ch->name = "Free#" + std::to_string(i);
		ch->maxplayer = 255;

		spdlog::get("console")->info("{}. {} MaxPlayer: {}", i, ch->name, ch->maxplayer);

		channel_list.push_back(ch);
	}
}

ChannelManager::~ChannelManager() 
{
	std::for_each(channel_list.begin(), channel_list.end(), [](Channel *ch) { delete ch; });
}

void ChannelManager::pc_req_enter_channel(pc* pc)
{
	uint8 channel_id = RTIU08(pc);
	Channel* ch = get_channel_ById(channel_id);

	if (ch == nullptr) {
		pc->disconnect();
		return;
	}

	// don't know what is it but offical server does
	if (pc->channel_ == nullptr) {
		Packet p;
		WTHEAD(&p, 0x95);
		WTIU08(&p, 2);
		WTIU16(&p, 1);
		pc->send(&p);
	}

	// if pc request the same old channel as he is in.
	if (pc->channel_ == ch) {
		return;
	}

	if (pc->channel_ != nullptr) {
		pc->channel_->pc_quit_lobby(pc);
	}

	if (ch->pc_count() >= ch->maxplayer) {
		send_channel_full(pc);
		return;
	}

	ch->pc_list.push_back(pc);
	pc->channel_ = ch;
	send_success(pc);
}

Channel* ChannelManager::get_channel_ById(uint8 id) 
{
	auto it = std::find_if(channel_list.begin(), channel_list.end(), [&id](Channel* m) {
		return m->id == id;
	});

	if ( it != std::end(channel_list) ) {
		return (*it);
	}

	return nullptr;
}

void ChannelManager::send_channel(pc* pc) 
{
	Packet p;
	WTHEAD(&p, 0x4D);
	WTIU08(&p, (uint8)channel_list.size());
	for (auto &it : channel_list) {
		WTFSTR(&p, it->name, 10);
		WTZERO(&p, 0x36);
		WTIU16(&p, it->maxplayer);
		WTIU16(&p, it->pc_count());
		WTIU08(&p, it->id);
		WTIU32(&p, 1);
		WTIU32(&p, 0);
	}
	pc->send(&p);
}

void ChannelManager::send_channel_full(pc* pc) 
{
	Packet p;
	WTHEAD(&p, 0x4E);
	WTIU08(&p, 2);
	pc->send(&p);
}

void ChannelManager::send_success(pc* pc) 
{
	Packet p;
	WTHEAD(&p, 0x4E);
	WTIU08(&p, 1);
	pc->send(&p);
}

void ChannelManager::getserver_data(pc* pc) 
{
	if (pc->channel_ == nullptr)
		return;

	Poco::Timespan diff = localtime() - lastcache;

	if (diff.minutes() > 5 || (uint8)serverlist_cache.size() == 0) {
		serverlist_cache.clear(); // Clear old lists

		Statement stm(*get_session());
		stm << "SELECT * FROM server WHERE server_type = 0", now;
		RecordSet rs(stm);

		bool done = rs.moveFirst();

		while (done) {
			std::unique_ptr<serverlist> sv = std::make_unique<serverlist>();
			sv->name = rs["name"].toString();
			sv->id = rs["server_id"];
			sv->ipaddr = rs["ip"].toString();
			sv->port = rs["port"];
			sv->imgevent = rs["img_event"];
			sv->serverimg = rs["img_no"];
			serverlist_cache.push_back(std::move(sv));
			done = rs.moveNext();
		}
		lastcache = localtime();
	}
	
	Packet p;
	WTHEAD(&p, 0x9F);
	WTIU08(&p, (uint8)serverlist_cache.size());

	for (auto &list : serverlist_cache) {
		WTFSTR(&p, list->name, 10);
		WTZERO(&p, 18);
		WTIU32(&p, 321005928); // MAYBE GAME VERSION
		WTZERO(&p, 8);
		WTIU32(&p, list->id);
		WTIU32(&p, 10000); // MAX PLAYER
		WTIU32(&p, 0); // ONLINE PLAYER
		WTFSTR(&p, list->ipaddr, 16);
		WTIU16(&p, 10592);
		WTIU16(&p, list->port);
		WTZERO(&p, 3);
		WTIU08(&p, 8);
		WTZERO(&p, 2);
		WTIU32(&p, 1); // ANGELIC NUMBER
		WTIU16(&p, list->imgevent);
		WTZERO(&p, 6);
		WTIU16(&p, list->serverimg);
	}

	WTIU08(&p, (uint8)channel_list.size());

	for (auto &it : channel_list) {
		WTFSTR(&p, it->name, 10);
		WTZERO(&p, 0x36);
		WTIU16(&p, it->maxplayer);
		WTIU16(&p, it->pc_count());
		WTIU08(&p, it->id);
		WTIU32(&p, 1);
		WTIU32(&p, 0);
	}

	pc->send(&p);
}

/* Channel */

Channel::Channel()
{
	addtimer(10000, std::bind(&Channel::game_destroy, this), true);
}

void Channel::game_destroy()
{
	for ( int i = 0; i < MAX_GAME_LIST; i++ ) {
		if ( game_list[i] != nullptr && !game_list[i]->valid ) {
			NULL_POINTER(game_list[i]);
		}
	}
}

void Channel::pc_enter_lobby(pc* pc) 
{
	sys_verify_pc(pc);

	if (pc->channel_in_) {
		return;
	}

	pc->channel_in_ = true;

	sys_send_pc_list(pc);
	sys_send_game_list(pc);
	sys_pc_action(pc, lbSend);
	sys_send_enter_lobby(pc);
}

void Channel::pc_leave_lobby(pc* pc) 
{
	sys_verify_pc(pc);

	if (!pc->channel_) {
		return;
	}

	pc->channel_in_ = false;
	sys_pc_action(pc, lbLeave);
	sys_send_leave_lobby(pc);
}

void Channel::pc_quit_lobby(pc* pc)
{
	sys_verify_pc(pc);

	auto it = std::find(pc_list.begin(), pc_list.end(), pc);

	if (it == std::end(pc_list)) {
		return;
	}

	if (pc->game != nullptr) {
		pc->game->pc_req_leave_game(pc);
	}

	if (pc->channel_in_) {
		sys_pc_action(pc, lbLeave);
	}

	pc_list.erase(it);

	pc->channel_ = nullptr;
	pc->channel_in_ = false;
}

void Channel::pc_req_chat(pc* sd)
{
	sys_verify_pc(sd);

	sd->read<uint32>();
	std::string name = RTSTR(sd);
	std::string msg = RTSTR(sd);

	if ( !STRCMP(sd->name_, name) ) {
		return;
	}

	Packet p;
	WTHEAD(&p, 0x40);
	WTIU08(&p, sd->capability_ == 4 ? 0x80 : 0);
	WTCSTR(&p, sd->name_);
	WTCSTR(&p, msg);

	if (sd->game == nullptr) {
		std::for_each(pc_list.begin(), pc_list.end(), [&p](pc* pc) {
			if (pc->game == nullptr && pc->channel_in_) {
				pc->send(&p);
			}
		});
	}
	else {
		sd->game->pc_chat(sd, &p);
	}
}

game* Channel::sys_getgame_byid(uint32 room_id)
{
	if (game_list[room_id] != nullptr) {
		return game_list[room_id];
	}

	return nullptr;
}

void Channel::sys_gm_command(pc* pc) {

	if (pc->capability_ != 4) throw "You're not not GM!";

	uint16 command = RTIU16(pc);

	switch (command) {
	case cmdWeather:
	{
		uint8 weather = RTIU08(pc);
		if (pc->game) {
			pc->game->sys_weather(weather);
		}
	}
	break;
	}
}

void Channel::sys_game_action(game* game, GAME_UPDATEACTION const& action) 
{
	sys_veriy_game(game);
	Packet p;
	WTHEAD(&p, 0x47);
	WTIU08(&p, 1);
	WTIU08(&p, (uint8)action);
	WTI16(&p, -1);
	game->roomdata(&p); // get game data
	sys_send(&p);
}

void Channel::sys_send_pc_list(pc* pc) 
{
	Packet p;
	WTHEAD(&p, 0x46);
	WTIU08(&p, 4); // Show Player

	int i = 0;

	for (auto &pc : pc_list) {
		if (pc->channel_in_) {
			i += 1;
		}
	}

	WTIU08(&p, i);

	for (auto pc : pc_list) {
		if (pc->channel_in_) {
			sys_get_pc_data(pc, &p);
		}
	}

	pc->send(&p);
}

void Channel::sys_send_game_list(pc* pc)
{
	int16 count = 0;

	for (int i = 0; i < MAX_GAME_LIST; ++i) {
		if (game_list[i] != nullptr && game_list[i]->valid) {
			count += 1;
		}
	}

	Packet p;
	WTHEAD(&p, 0x47);
	WTIU16(&p, count);
	WTI16(&p, -1);

	for (int i = 0; i < MAX_GAME_LIST; ++i) {
		if (game_list[i] != nullptr && game_list[i]->valid) {
			game_list[i]->roomdata(&p);
		}
	}

	pc->send(&p);
}

void Channel::sys_pc_action(pc* pc, PC_GAMEACTION const& action) 
{
	Packet p;
	WTHEAD(&p, 0x46);
	WTIU08(&p, (uint8)action);
	WTIU08(&p, 1);
	sys_get_pc_data(pc, &p);
	sys_send(&p);
}

void Channel::sys_get_pc_data(pc* pc, Packet* p) 
{
	WTIU32(p, pc->account_id_);
	WTIU32(p, pc->connection_id_);
	WTI16(p, pc->game_id);
	WTFSTR(p, pc->name_, 16);
	WTZERO(p, 6);
	WTIU08(p, pc->state->level); // LEVEL
	WTIU32(p, 0); // GM VISIBLE
	WTIU32(p, 0); // TITLE TYPEID
	WTIU32(p, 1000); // WHAT??
	WTIU08(p, pc->sex_);
	WTIU32(p, 0); // GUILD ID
	WTFSTR(p, "", 9); // GUILD IMAGE
	WTZERO(p, 3);
	WTIU08(p, 1); // VIP
	WTZERO(p, 6);
	WTFSTR(p, pc->username_ + "@NT", 18);
	WTZERO(p, 0x6E);
}

void Channel::sys_send_enter_lobby(pc* pc) 
{
	Packet p;
	WTHEAD(&p, 0xF5);
	pc->send(&p);
}

void Channel::sys_send_leave_lobby(pc* pc) 
{
	Packet p;
	WTHEAD(&p, 0xF6);
	pc->send(&p);
}

void Channel::sys_send(Packet* packet) 
{
	std::for_each(pc_list.begin(), pc_list.end(), [&packet](pc* pc) {
		pc->send(packet);
	});
}

void Channel::sys_verify_pc(pc* pc) 
{
	if (pc == nullptr)
		return throw ChannelNotFound();

	if ( std::find(std::begin(pc_list), std::end(pc_list), pc) == std::end(pc_list) )
		throw ChannelNotFound();
}

void Channel::sys_veriy_game(game* game)
{
	if (game == nullptr)
		return throw GameNotFoundOnChannel();

	int game_id = game->roomId;

	if (game_list[game_id] != game) {
		throw GameNotFoundOnChannel();
	}
}

int Channel::acquire_gameid() {
	for (int i = 0; i < MAX_GAME_LIST; ++i) {
		if (game_list[i] == nullptr) {
			return i;
		}
	}

	return -1;
}

void Channel::pc_req_create_game(pc* pc)
{
	if (pc->game != nullptr) { // If this player still in any rooms.
		pc->game->pc_req_leave_game(pc);
		send_room_error(pc, rFail);
		return;
	}

	std::shared_ptr<gamedata> data = CREATE_SHARED(gamedata);
	RTPOINTER(pc, &data->un1, sizeof gamedata);

	printf("vs = %d | match = %d | hole total = %d | max player %d | type = %d \n", data->vs_time, data->match_time, data->hole_total, data->max_player, data->game_type);

	int game_id = acquire_gameid();

	if (game_id == -1)
		return; // Not enough game slot

	switch (data->game_type) {
	case gtStroke:
	{
		uint32 natural = RTIU32(pc);
		std::string name = RTSTR(pc);
		std::string pwd = RTSTR(pc);
		uint32 artifact = RTIU32(pc);
		game_list[game_id] = new game_stroke(data, game_id, name, pwd);
		game_list[game_id]->natural = natural;
	}
	break;
	case gtChatroom:
	{
		uint32 natural = RTIU32(pc);
		std::string name = RTSTR(pc);
		std::string pwd = RTSTR(pc);
		uint32 artifact = RTIU32(pc);
		game_list[game_id] = new game_chatroom(data, game_id, name, pwd);
		game_list[game_id]->natural = natural;
	}
	break;
	case gtPractice:
	{
		holepractice practicedata;
		holerepeat repeatdata;
		if (pc->readstruct((char*)&practicedata.natural, sizeof(holepractice))) {
			game_list[game_id] = new game_practice(data, practicedata, game_id);
		}
		else if (pc->readstruct((char*)&repeatdata.holepos, sizeof(holerepeat))) {
			game_list[game_id] = new game_practice(data, repeatdata, game_id);
		}
		else {
			return;
		}
	}
	break;
	}

	if (!game_list[game_id]) {
		send_room_error(pc, rFail);
		return;
	}

	game_list[game_id]->channel = this;
	game_list[game_id]->addmaster(pc);
}

void Channel::pc_req_join_game(pc* pc) {
	uint16 req_id = RTIU16(pc);
	std::string req_pwd = RTSTR(pc);
	game* game = sys_getgame_byid(req_id);

	if (game == nullptr || !game->valid) {
		send_room_error(pc, rNotExist);
		return;
	}

	if ((uint8)game->pc_list.size() >= game->maxplayer) {
		send_room_error(pc, rFull);
		return;
	}

	// if password not match or not gm
	if (game->password.compare(req_pwd) != 0 && pc->capability_ != 4) {
		send_room_error(pc, rPwdErr);
		return;
	}

	game->addpc(pc);
}

void Channel::send_pc_leave_game(pc* pc) {
	Packet p;
	WTHEAD(&p, 0x4C);
	WTI16(&p, -1);
	pc->send(&p);
}

void Channel::send_room_error(pc* pc, roomErr err) {
	Packet p;
	WTHEAD(&p, 0x49);
	WTIU08(&p, (uint8)err);
	pc->send(&p);
}