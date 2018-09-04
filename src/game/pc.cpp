#include "pc.h"
#include "account.h"
#include "channel.h"
#include "shop.h"
#include "item.h"
#include "mail.h"

#include "Poco/Data/ODBC/ODBCException.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "../common/packet.h"
#include "../common/asio.hpp"

pc::pc(int con_id, Session *session) :
	connection_id_(con_id), session_(session),
	inventory(new Inventory()), account_id_(0),
	warehouse(std::make_shared<PC_Warehouse>())
{}

pc::~pc() { 

	if (channel_) {
		channel_->pc_quit_lobby(this);
		channel_ = nullptr;
	}

	spdlog::get("console")->warn("PC {} disconnected!", get_connection_id()); 
	delete inventory;
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

void pc::send_packet(Packet *packet) {
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

	__int16 packet_id = read<__int16>();

	for (int i = 0; i < bytes_recv - 5; ++i) {
		printf("%02x ", *(unsigned __int8*)(session_->get_receive_buffer() + i + 5));
	}
	printf("\n");

	try {
		switch (packet_id) {
		case packet::pc_send_message:
			channel_->pc_send_message(this);
			break;
		case packet::pc_login:
			pc_process->pc_login(this);
			break;
		case packet::pc_select_channel:
			channel_manager->pc_select_channel(this);
			break;
		case packet::pc_enter_lobby_:
			channel_->pc_enter_lobby(this);
			break;
		case packet::pc_leave_lobby_:
			channel_->pc_leave_lobby(this);
			break;
		case packet::pc_enter_shop:
			shop->pc_entershop(this);
			break;
		case packet::pc_buyitem:
			shop->pc_buyitem(this);
			break;
		case packet::pc_open_cardpack:
			pc_opencardpack(this);
			break;
		case packet::pc_loadmail_:
			pc_loadmail(this);
			break;
		case packet::pc_readmail_:
			pc_readmail(this);
			break;
		default:
			break;
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

}

void pc::sync_money() {
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
}