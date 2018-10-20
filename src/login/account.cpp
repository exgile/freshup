#include "account.h"
#include "pc.h"
#include "clif.h"
#include "pcs.h"

#include "../common/typedef.h"
#include "../common/utils.h"
#include "../common/db.h"
#include "../common/packet.h"

using namespace Poco::Data::Keywords;
using namespace Poco::Data;

account* pc_process = nullptr;

account::account(){}
account::~account(){}

void account::pc_login(pc* pc) {
	std::string username = RTSTR(pc);
	std::string password = RTSTR(pc);

	Poco::Data::Session sess = sdb->get_session();
	Poco::Data::Statement query(sess);
	query << "SELECT account_id, userid, user_pass, state, name, FirstSet FROM account WHERE userid = ?", use(username), now;
	Poco::Data::RecordSet rs(query);

	// USER NOT FOUND
	if ( (rs.rowCount() <= 0) || !STRCMP(username, rs["userid"].toString())) {
		sclif->login_msg(pc, USER_NOT_FOUND);
		pc->disconnect();
		return;
	}
	
	// PASSWORD ERROR
	if ( !STRCMP(password, rs["user_pass"].toString()) ) {
		sclif->login_msg(pc, INCORRECT_PASSWORD);
		pc->disconnect();
		return;
	}

	// USER BANNED
	if (rs["state"] > 0) {
		sclif->login_msg(pc, USER_STATE_BAN);
		pc->disconnect();
		return;
	}

	pc->set_accountid(rs["account_id"]);
	pc->set_username(rs["userid"]);
	pc->set_name(rs["name"]);

	sclif->send_accountid(pc);

	if (rs["FirstSet"] == 0) {
		sclif->send_firstset(pc);
		return;
	}

	sys_send_data(pc);
}

// return true : name is not exist
// return false : name is already existed
bool account::sys_name_validation(std::string& name) {
	Poco::Data::Session sess = sdb->get_session();
	Poco::Data::Statement stm(sess);
	stm << "SELECT 1 FROM account WHERE name = ?", use(name), now;
	Poco::Data::RecordSet rs(stm);

	if (rs.rowCount() <= 0) {
		return true;
	}

	return false;
}

void account::pc_checkup_name(pc* pc) {
	std::string name = RTSTR(pc);

	if (sys_name_validation(name)) {
		sclif->send_name_available(pc, name);
		return;
	}

	sclif->send_name_taken(pc);
}

void account::pc_name_validation(pc* pc) {
	std::string name = RTSTR(pc);

	if (sys_name_validation(name)) {
		pc->set_name(name);
		sclif->send_name_validate_true(pc);
		return;
	}

	sclif->send_name_validate_failed(pc);
}

void account::pc_request_create_char(pc* pc) {
	uint32 char_typeid = RTIU32(pc);
	uint16 hair_colour = RTIU16(pc);

	Poco::Data::Session sess = sdb->get_session();
	Poco::Data::Statement stm(sess);
	stm << "EXEC sys_firstset ?, ?, ?, ?", bind(pc->get_accountid()), bind(pc->get_name()), use(char_typeid), use(hair_colour), now;
	Poco::Data::RecordSet rs(stm);

	// error occurred
	if (rs["ERROR_CODE"] > 0) {
		pc->disconnect();
		return;
	}

	Packet p;
	WTHEAD(&p, 0x11);
	WTIU08(&p, 0);
	pc->send_packet(&p);

	sys_send_data(pc);
}

void account::sys_send_data(pc* pc) {
	std::string game_key = rnd_str(7);
	std::string login_key = rnd_str(7);

	Poco::Data::Session sess = sdb->get_session();
	Poco::Data::Statement stm(sess);
	stm << "UPDATE account SET game_key = ?, login_key = ?, logincount += 1, lastlogin = GETDATE() WHERE account_id = ?", 
		use(game_key), use(login_key), bind(pc->get_accountid()), now;

	pc->login_key = login_key;
	pc->game_key = game_key;

	sclif->send_auth_login(pc);
	sclif->send_pc_data(pc);
	sclif->send_game_server(pc);

	// TODO: should send messenger server

	sclif->send_hotkey(pc);
}

void account::pc_request_gamekey(pc* pc) {
	Packet p;
	WTHEAD(&p, 3);
	WTIU32(&p, 0);
	WTCSTR(&p, pc->game_key);
	pc->send_packet(&p);
}