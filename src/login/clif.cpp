#include "clif.h"
#include "pc.h"
#include "static.h"
#include "../common/typedef.h"
#include "../common/db.h"
#include "../common/packet.h"

clif* sclif = nullptr;

clif::clif() {}
clif::~clif() {}

void clif::login_msg(pc* pc, loginmsg type) {
	Packet packet;

	switch(type) {
	case INCORRECT_PASSWORD:
		packet.write_hex(&INCORRECT_PWD[0], sizeof(INCORRECT_PWD));
		break;
	case USER_NOT_FOUND:
		packet.write_hex(&USER_NOTFOUND[0], sizeof(USER_NOTFOUND));
		break;
	case USER_STATE_BAN:
		packet.write_hex(&USERGETBANNED[0], sizeof(USERGETBANNED));
		break;
	case MAINTENANCE_MODE:
		packet.write_hex(&SVMAINTENANCE[0], sizeof(SVMAINTENANCE));
		break;
	}

	pc->send_packet(&packet);
}

void clif::send_accountid(pc* pc) {
	Packet packet;
	packet.write<__int16>(0x03);
	packet.write<__int32>(pc->get_accountid());
	pc->send_packet(&packet);
}

void clif::send_firstset(pc* pc) {
	Packet packet;
	packet.write<__int16>(0x0f);
	packet.write<__int8>(0);
	packet.write<std::string>(pc->get_username());
	pc->send_packet(&packet);

	packet.reset();

	packet.write_hex(&FIRSTSET[0], sizeof(FIRSTSET));
	pc->send_packet(&packet);
}

void clif::send_name_taken(pc* pc) {
	Packet packet;
	packet.write<__int16>(0x0e);
	packet.write<__int32>(0x0c);
	packet.write<__int32>(5100065); // code that show the name is taken
	pc->send_packet(&packet);
}

void clif::send_name_available(pc* pc, std::string& name) {
	Packet packet;
	packet.write<__int16>(0x0e);
	packet.write<__int32>(0);
	packet.write<std::string>(name);
	pc->send_packet(&packet);
}

void clif::send_name_validate_true(pc* pc) {
	Packet packet;
	packet.write<__int16>(1);
	packet.write<uint8>(0xda);
	pc->send_packet(&packet);
}

void clif::send_name_validate_failed(pc* pc) {
	Packet packet;
	packet.write<__int16>(0x0d);
	packet.write<__int32>(0x0c);
	packet.write<__int32>(5100049);
	pc->send_packet(&packet);
}

void clif::send_auth_login(pc* pc) {
	Packet packet;
	packet.write<uint16>(0x10);
	packet.write<std::string>(pc->login_key);
	pc->send_packet(&packet);
}

void clif::send_pc_data(pc* pc) {
	Packet packet;
	packet.write<uint16>(1);
	packet.write<uint8>(0);
	packet.write<std::string>(pc->get_username());
	packet.write<uint32>(pc->get_accountid());
	packet.write<uint32>(0);
	packet.write<uint32>(0); // level
	packet.write_null(6);
	packet.write<std::string>(pc->get_name());
	pc->send_packet(&packet);
}

void clif::send_game_server(pc* pc) {
	Poco::Data::Session sess = sdb->get_session();
	Poco::Data::Statement stm(sess);
	stm << "SELECT * FROM server WHERE server_type = 0", Poco::Data::Keywords::now;
	Poco::Data::RecordSet rs(stm);
	bool done = rs.moveFirst();

	Packet packet;
	packet.write<uint16>(2);
	packet.write<uint8>((uint8)rs.rowCount());

	while (done) {
		packet.write_string(rs["name"] , 10);
		packet.write_null(18);
		packet.write<uint32>(321005928); // maybe game version
		packet.write_null(8);
		packet.write<uint32>(rs["server_id"]);
		packet.write<uint32>(5000); // max player
		packet.write<uint32>(0); // online player
		packet.write_string(rs["ip"], 16);
		packet.write<uint16>(10592); // ???
		packet.write<uint16>(rs["port"]);
		packet.write_null(3);
		packet.write<uint8>(8);
		packet.write_null(2);
		packet.write<uint32>(1); // angelic number
		packet.write<uint16>(rs["img_event"]);
		packet.write_null(6);
		packet.write<uint16>(rs["img_no"]);
		done = rs.moveNext();
	}

	pc->send_packet(&packet);
}

void clif::send_hotkey(pc* pc) {
	Poco::Data::Session sess = sdb->get_session();
	Poco::Data::Statement stm(sess);
	stm << "SELECT * FROM hotkey WHERE account_id = ?", Poco::Data::Keywords::bind(pc->get_accountid()), Poco::Data::Keywords::now;
	Poco::Data::RecordSet rs(stm);
	bool done = rs.moveFirst();

	Packet packet;
	packet.write<uint16>(6);
	while (done) {
		packet.write_string(rs["hotkey1"] , 64);
		packet.write_string(rs["hotkey2"], 64);
		packet.write_string(rs["hotkey3"], 64);
		packet.write_string(rs["hotkey4"], 64);
		packet.write_string(rs["hotkey5"], 64);
		packet.write_string(rs["hotkey6"], 64);
		packet.write_string(rs["hotkey7"], 64);
		packet.write_string(rs["hotkey8"], 64);
		packet.write_string(rs["hotkey9"], 64);
		done = rs.moveNext();
	}

	pc->send_packet(&packet);
}