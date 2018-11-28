#include "account.h"
#include "pc.h"
#include "reader.h"
#include "channel.h"
#include "static.h"

#include "../common/db.h"
#include "../common/packet.h"
#include "../common/typedef.h"

void pc_req_login(pc* pc) {
	std::string username = RTSTR(pc);
	int account_id = RTIU32(pc);
	RSKIP(pc, 6);
	std::string login_key = RTSTR(pc);
	std::string game_ver = RTSTR(pc);
	RSKIP(pc, 8);
	std::string game_key = RTSTR(pc);

	Packet p;

	Statement stm(*get_session());
	stm << "SELECT account_id, userid, name, capability, sex, cookie FROM account WHERE account_id=? AND userid=? AND login_key=? AND game_key=?", 
		use(account_id), use(username), use(login_key), use(game_key), now;
	RecordSet rs(stm);
	
	if (rs.rowCount() <= 0) {
		Packet p;
		WTHEAD(&p, 630);
		WTIU32(&p, 300);
		pc->send(&p);
		pc->disconnect();
		return;
	}

	pc->login_key = login_key;
	pc->game_key = game_key;
	pc->account_id_ = rs["account_id"];
	pc->username_ = rs["userid"].toString();
	pc->name_ = rs["name"].toString();
	pc->capability_ = rs["capability"];
	pc->sex_ = rs["sex"];
	pc->cookie = rs["cookie"];

	// load
	pc->load_statistics();
	pc_load_data(pc);

	sys_send_jerk(pc);
	
	sys_send_pc_data(pc);
	chm->send_channel(pc);

	pc_sync_data(pc, IV_CHAR);
	pc_sync_data(pc, IV_ALLITEM);
	pc_sync_data(pc, IV_EQUIPMENT);

	WTHEAD(&p, 0xF1);
	WTIU08(&p, 0);
	pc->send(&p);
	
	p.reset();
	WTHEAD(&p, 0x135);
	pc->send(&p);

	pc_sync_data(pc, IV_CARD);

	pc->sendcookie();
}

void sys_send_pc_data(pc* pc) {
	Packet p;
	WTHEAD(&p, 0x44);
	WTIU08(&p, 0);
	WTCSTR(&p, config->Get("server", "client_Ver", "829.01"));
	WTIU16(&p, 65535);
	WTFSTR(&p, pc->username_, 15);
	WTZERO(&p, 7);
	WTFSTR(&p, pc->name_, 16);
	WTZERO(&p, 6);
	WTZERO(&p, 21); // GUILD NAME
	WTZERO(&p, 9); // GUILD IMAGE
	WTZERO(&p, 7);
	WTIU32(&p, pc->capability_);
	WTIU32(&p, 0);
	WTIU32(&p, pc->connection_id_);
	WTZERO(&p, 12);
	WTIU32(&p, 0); // GUILD ID
	WTPOINTER(&p, &pc_data1[0], sizeof(pc_data1));
	WTFSTR(&p, pc->username_ + "@NT", 18);
	WTZERO(&p, 0x6E);
	WTIU32(&p, pc->account_id_);
	WTPOINTER(&p, pc->state.get(), sizeof(statistics)); // TODO: statistic
	WTPOINTER(&p, &pc_data2[0], sizeof(pc_data2));
	p.write_datetime();
	WTPOINTER(&p, &pc_data3[0], sizeof(pc_data3));
	WTZERO(&p, 8);
	WTIU08(&p, 1);
	WTIU32(&p, 0);
	WTIU08(&p, 8); // grandprix? 0 = no grandprix
	WTIU16(&p, 0);

	WTIU32(&p, 0); // GUILD ID
	WTFSTR(&p, "", 20);
	WTZERO(&p, 9);
	WTIU32(&p, 0); // GUILD TOTAL MEMBERS
	WTFSTR(&p, "", 9); // GUILD IMAGE TO FILE
	WTZERO(&p, 3);
	WTFSTR(&p, "", 0x65); // GUILD NOTICE
	WTFSTR(&p, "", 101); // GUILD INTRODUCE
	WTIU32(&p, 0); // MY CURRENT POSITION
	WTIU32(&p, 0); // LEADER ACCOUNT ID
	WTFSTR(&p, "", 22); // LEADER NICKNAME

	pc->send(&p);
}

void sys_send_jerk(pc* pc) {
	Packet packet;
	packet.write_hex(&game_login1[0], sizeof(game_login1));
	pc->send(&packet);

	packet.reset();
	packet.write_hex(&game_login2[0], sizeof(game_login2));
	pc->send(&packet);

	packet.reset();
	packet.write_hex(&game_login3[0], sizeof(game_login3));
	pc->send(&packet);

	packet.reset();
	packet.write_hex(&game_login4[0], sizeof(game_login4));
	pc->send(&packet);

	packet.reset();
	packet.write_hex(&game_login5[0], sizeof(game_login5));
	pc->send(&packet);

	packet.reset();
	packet.write_hex(&game_login6[0], sizeof(game_login6));
	pc->send(&packet);

	packet.reset();
	packet.write_hex(&game_login7[0], sizeof(game_login7));
	pc->send(&packet);

	packet.reset();
	packet.write_hex(&game_login8[0], sizeof(game_login8));
	pc->send(&packet);

	packet.reset();
	packet.write_hex(&game_login9[0], sizeof(game_login9));
	pc->send(&packet);

	packet.reset();
	packet.write_hex(&game_login10[0], sizeof(game_login10));
	pc->send(&packet);

	packet.reset();
	packet.write_hex(&game_login11[0], sizeof(game_login11));
	pc->send(&packet);

	packet.reset();
	packet.write_hex(&game_login12[0], sizeof(game_login12));
	pc->send(&packet);

	packet.reset();
	packet.write_hex(&game_login13[0], sizeof(game_login13));
	pc->send(&packet);

	packet.reset();
	packet.write_hex(&game_login14[0], sizeof(game_login14));
	pc->send(&packet);

	packet.reset();
	packet.write_hex(&game_login15[0], sizeof(game_login15));
	pc->send(&packet);

	packet.reset();
	packet.write_hex(&game_login16[0], sizeof(game_login16));
	pc->send(&packet);

	packet.reset();
	packet.write_hex(&game_login17[0], sizeof(game_login17));
	pc->send(&packet);

	packet.reset();
	packet.write_hex(&game_login18[0], sizeof(game_login18));
	pc->send(&packet);

	packet.reset();
	packet.write_hex(&game_login19[0], sizeof(game_login19));
	pc->send(&packet);
}

void pc_req_savedata(pc *pc) {
	uint8 type = RTIU08(pc);

	Packet p;
	WTHEAD(&p, 0x6B);
	WTIU08(&p, 4);
	WTIU08(&p, type);

	switch (type) {
	case 2:
	{
		RTPOINTER(pc, &pc->itemuse, sizeof(pc->itemuse));
		WTPOINTER(&p, &pc->itemuse, sizeof(pc->itemuse));
	}
	break;
	}
	
	pc->send(&p);
}