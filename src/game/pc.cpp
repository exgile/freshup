#include "pc.h"
#include "channel.h"
#include "packetdb.h"
#include "gameplay.h"

#include "Poco/Data/ODBC/ODBCException.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "../common/packet.h"
#include "../common/db.h"

pc::pc(int con_id, Session *session) :
	connection_id_(con_id), 
	session_(session),
	account_id_(0),
	channel_(nullptr),
	game(nullptr),
	state(std::make_unique<statistics>()),
	equipment(std::make_shared<PC_Equipment>())
{}

pc::~pc() { 
	// Save inventory data
	pc_savedata(this);

	// To do:: to everything inside channel class
	if (channel_ != nullptr) {
		channel_->pc_quit_lobby(this);
	}

	spdlog::get("console")->warn("PC {} disconnected!", get_connection_id()); 
}

int pc::get_connection_id() {
	return connection_id_;
}

void pc::skip(int amount) {
	recv_pos_ += amount;
}

void pc::disconnect() {
	session_->disconnect();
}

void pc::send(Packet *packet) {
	session_->send_packet(packet);
}

void pc::send_packet_undecrypt(Packet *packet) {
	session_->send_packet_undecrypt(packet);
}

void pc::handle_packet(unsigned short bytes_recv) {
	recv_pos_ = 0;
	recv_length_ = bytes_recv;

	// skip unuse packets
	skip(5);

	uint16 packet_id = read<uint16>();

	for (int i = 0; i < bytes_recv - 5; ++i) {
		printf("%02x ", *(uint8*)(session_->get_receive_buffer() + i + 5));
	}
	printf("\n");

	Poco::DateTime now;

	try {
		auto db = packetdb.find(packet_id);

		if (db != packetdb.end()) {
			db->second(this);
		}
		else {
			spdlog::get("console")->critical("Packet ID : {} Not Found on database", packet_id);
		}
	}
	catch (Poco::Data::ODBC::ODBCException& e) {
		spdlog::get("console")->critical(e.message());
		disconnect();
	}
	catch (ChannelNotFound& e) {
		spdlog::get("console")->critical(e.what());
	}
	catch (std::exception& e) {
		spdlog::get("console")->critical(e.what());
		disconnect();
	}
	catch (...) {
		spdlog::get("console")->critical("Unknown Exception Occured!");
		disconnect();
	}

	Poco::DateTime due;
	Poco::Timespan diff = due - now;
	spdlog::get("console")->info("PC last processed took {}ms", diff.milliseconds());

}

void pc::gamedata(Packet* p, bool with_equip) {
	WTIU32(p, connection_id_);
	WTFSTR(p, name_, 0x10);
	WTZERO(p, 6);
	WTFSTR(p, "", 0x15); // guild name
	WTIU08(p, game_slot);
	WTIU32(p, 0);
	WTIU32(p, 0); // title typeid
	WTIU32(p, char_typeid_equiped(this));
	WTZERO(p, 0x14); // ?
	WTIU32(p, 0); // title typeid
	WTIU08(p, game_role);
	WTIU08(p, game_ready ? 2 : 0);
	WTIU08(p, state->level); // level
	WTIU08(p, 0); // GM
	WTIU08(p, 10); // ??
	WTIU32(p, 0); // GUILD ID
	WTFSTR(p, "guildmark", 9);
	WTIU32(p, 0);
	WTIU32(p, account_id_);
	WTIU32(p, animate);
	WTIU16(p, 41485);
	WTIU32(p, posture);
	WTFLO(p, game_position.x);
	WTFLO(p, game_position.y);
	WTFLO(p, game_position.z);
	WTIU32(p, 0);
	WTFSTR(p, "STORE NAME", 0x1F);
	WTZERO(p, 0x21);
	WTIU32(p, 0); // MASCOT TYPEID
	WTIU08(p, 0); // PANG MAS ENABLE
	WTIU08(p, 0); // NITRO PANG MAS ENABLE
	WTIU32(p, 0); // ?
	WTFSTR(p, username_ + "@NT", 18);
	WTZERO(p, 0x6E);
	WTIU32(p, 0); // THOPHY ?
	WTIU32(p, 66); // ??

	if (with_equip) {
		write_current_char(this, p);
	}
}
void pc::change_equipment()
{
	Packet p;
	uint8 action = RTIU08(this);

	WTHEAD(&p, 0x4B);
	WTIU32(&p, 0);
	WTIU08(&p, action);
	WTIU32(&p, connection_id_);

	switch (action) {
	case eqChar:
	{
		uint32 charid = RTIU32(this);
		if (!setchar(this, charid)) {
			return;
		}
		write_current_char(this, &p);
		if (game != nullptr) {
			game->send(&p);
		}
	}
	break;
	case eqstart:
	{
		if (game != nullptr) {
			game->pc_req_gamedata(this);
		}
	}
	break;
	}
}

bool pc::removepang(int amount) {
	if (state->pang < amount)
		return false;

	state->pang -= amount;
	return true;
}

bool pc::removecookie(int amount) {
	if (cookie < amount)
		return false;

	cookie -= amount;
	return true;
}

void pc::sendpang() {
	Packet p;
	WTHEAD(&p, 0xc8);
	WTIU64(&p, state->pang);
	WTIU64(&p, 0);
	send(&p);
}

void pc::sendcookie() {
	Packet p;
	WTHEAD(&p, 0x96);
	WTIU64(&p, cookie);
	send(&p);
}

void pc::transaction_push(uint8 key, ITEM_TRANSACTION tran) {
	transaction.push_back(std::make_pair(key, tran));
}

void pc::transaction_sync() {
	Packet p;
	Poco::DateTime dt = localtime();
 
	WTHEAD(&p, 0x216);
	WTIU32(&p, timestamp());
	WTIU32(&p, (uint32)transaction.size());

	for (auto &it : transaction) 
	{
		WTIU08(&p, it.first); // key
		WTIU32(&p, it.second->item_typeid); // Item Typeid
		WTIU32(&p, it.second->item_id); // Item Typeid

		if (it.first == 0xC9) // club powder
		{
			WTIU32(&p, 0);
			WTIU32(&p, 0);
			WTIU32(&p, 0);
			WTIU32(&p, 0);
			WTIU16(&p, club_data_[it.second->item_id]->c0);
			WTIU16(&p, club_data_[it.second->item_id]->c1);
			WTIU16(&p, club_data_[it.second->item_id]->c2);
			WTIU16(&p, club_data_[it.second->item_id]->c3);
			WTIU16(&p, club_data_[it.second->item_id]->c4);
			WTZERO(&p, 0xF);
		}
		else if (it.first == 0xCB) // Put card to character 
		{
			WTIU32(&p, 0);
			WTIU32(&p, 0);
			WTIU32(&p, 0);
			WTIU32(&p, 0);
			WTZERO(&p, 20);
			WTIU32(&p, it.second->card_typeid);
			WTIU08(&p, it.second->char_slot);
		}
		else if (it.first == 0xCC) // Club Enhancement
		{
			WTIU32(&p, 0);
			WTIU32(&p, 0);
			WTIU32(&p, 0);
			WTIU32(&p, 0);
			WTZERO(&p, 20);

			WTIU32(&p, 0);
			WTIU08(&p, 0);
			WTIU16(&p, club_data_[it.second->item_id]->c0);
			WTIU16(&p, club_data_[it.second->item_id]->c1);
			WTIU16(&p, club_data_[it.second->item_id]->c2);
			WTIU16(&p, club_data_[it.second->item_id]->c3);
			WTIU16(&p, club_data_[it.second->item_id]->c4);
			WTIU32(&p, it.second->club_point);
			WTIU08(&p, it.second->club_count > 0 ? 0 : 0xFF);
			WTIU32(&p, it.second->club_count);
			WTIU32(&p, it.second->club_cancelcount);
		}
		else {
			Poco::Timespan diff = it.second->date_end - dt;

			WTIU32(&p, diff.hours() >= 1 ? 1 : 0);
			WTIU32(&p, diff.hours() >= 1 ? timestamp(it.second->date_reg) : it.second->old_amount);
			WTIU32(&p, diff.hours() >= 1 ? timestamp(it.second->date_end) : it.second->new_amount);
			WTIU32(&p, diff.hours() >= 1 ? diff.days() : (int) (it.second->new_amount - it.second->old_amount));
			WTZERO(&p, 8);
			WTIU16(&p, diff.hours() >= 1 ? diff.days() : 0);
			WTIU16(&p, (uint16)it.second->ucc_unique.length());
			WTFSTR(&p, it.second->ucc_unique, 0x8);
			// TODO : UCC
			WTIU32(&p, 0);
			WTIU08(&p, 0);
		}
	}
	send(&p);
	transaction.clear();
}

void pc::load_statistics() {
	Statement stm(*get_session());
	stm << "SELECT * FROM statistic WHERE UID = ?", use(account_id_), now;
	RecordSet rs(stm);

	if ((int)rs.rowCount()) {
		state->drive = rs["Drive"];
		state->putt = rs["Putt"];
		state->playertime = rs["Playtime"];
		state->longestdistance = rs["Longest"];
		state->distancetotal = rs["Distance"];
		state->pangya = rs["Pangya"];
		state->hole = rs["Hole"];
		state->teamhole = rs["TeamHole"];
		state->holeinone = rs["Holeinone"];
		state->ob = rs["OB"];
		state->bunker = rs["Bunker"];
		state->fairway = rs["Fairway"];
		state->albratoss = rs["Albatross"];
		state->holein = rs["Holein"];
		state->pang = rs["Pang"];
		state->timeout = rs["Timeout"];
		state->level = rs["Game_Level"];
		state->exp = rs["Game_Point"];
		state->puttin = rs["PuttIn"];
		state->longestputt = rs["LongestPuttIn"];
		state->longestchip = rs["LongestChipIn"];
		state->nomannergamecount = rs["NoMannerGameCount"];
		state->shottime = rs["ShotTime"];
		state->gameplayed = rs["GameCount"];
		state->disconnected = rs["DisconnectGames"];
		state->teamwin = rs["wTeamWin"];
		state->teamgame = rs["wTeamGames"];
		state->ladderpoint = rs["LadderPoint"];
		state->ladderwin = rs["LadderWin"];
		state->ladderlose = rs["LadderLose"];
		state->ladderdraw = rs["LadderDraw"];
		state->combocount = rs["ComboCount"];
		state->maxcombo = rs["MaxComboCount"];
		state->totalscore = rs["TotalScore"];
		state->score[0] = rs["BestScore0"];
		state->score[1] = rs["BestScore1"];
		state->score[2] = rs["BestScore2"];
		state->score[3] = rs["BestScore3"];
		state->score[4] = rs["BestScore4"];
		state->maxpang[0] = rs["MaxPang0"];
		state->maxpang[1] = rs["MaxPang1"];
		state->maxpang[2] = rs["MaxPang2"];
		state->maxpang[3] = rs["MaxPang3"];
		state->maxpang[4] = rs["MaxPang4"];
		state->sumpang = rs["SumPang"];
		state->ladderhole = rs["LadderHole"];
		state->gamecountseason = rs["GameCountSeason"];
		state->skinspang = rs["SkinsPang"];
		state->skinswin = rs["SkinsWin"];
		state->skinslose = rs["SkinsLose"];
		state->skinsrunhole = rs["SkinsRunHoles"];
		state->skinsstrikepoint = rs["SkinsStrikePoint"];
		state->skinsallincount = rs["SkinsAllinCount"];
	}
	else {
		disconnect();
	}
}

/*void pc::sync_money() {
	Packet packet;
	packet.write<uint16>(0xc8);
	packet.write<uint32>(100000000);
	packet.write_null(12);
	send_packet(&packet);

	packet.reset();
	packet.write<uint16>(0x96);
	packet.write<uint32>(100000000);
	packet.write<uint32>(0);
	send_packet(&packet);
}*/