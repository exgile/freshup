#include "channel.h"
#include "pc.h"
#include "../common/packet.h"
#include "spdlog/sinks/stdout_color_sinks.h"

ChannelManager* channel_manager = nullptr;

ChannelManager::ChannelManager() {
	/* Create Channel Lists */
	for (uint8 i = 1; i <= 4; ++i) {
		Channel* ch_data(new Channel());
		ch_data->id = i;
		ch_data->name = "Free#" + std::to_string(i);
		ch_data->maxplayer = 255;
		/* Show to console */
		spdlog::get("console")->info("{}. {} MaxPlayer: {}", i, ch_data->name, ch_data->maxplayer);
		/* Put to the vector */
		channel_list.push_back(ch_data);
	}
}

ChannelManager::~ChannelManager() {
	for_each(channel_list.begin(), channel_list.end(), [](Channel* ch) -> void { delete ch; });
}

void ChannelManager::pc_select_channel(pc* pc) {
	uint8 channel_id = pc->read<uint8>();
	Channel* ch = get_channel_ById(channel_id);
	
	if (ch == nullptr) {
		pc->disconnect();
		return;
	}

	if (ch->pc_count() >= ch->maxplayer) {
		send_channel_full(pc);
		return;
	}

	ch->pc_list.push_back(pc);
	pc->channel_ = ch;
	send_success(pc);
}

Channel* ChannelManager::get_channel_ById(uint8 id) {
	auto it = std::find_if(channel_list.begin(), channel_list.end(), [&id](Channel* m) {
		return m->id == id; 
	});
	
	if (it != channel_list.end()) {
		return (*it);
	}

	return nullptr;
}

void ChannelManager::send_channel(pc* pc){
	Packet packet;
	packet.write<uint16>(0x4d);
	packet.write<uint8>((uint8)channel_list.size());
	for (auto it : channel_list) {
		packet.write_string(it->name, 10);
		packet.write_null(0x36);
		packet.write<uint16>(it->maxplayer);
		packet.write<uint16>(it->pc_count());
		packet.write<uint8>(it->id);
		packet.write<uint32>(1);
		packet.write<uint32>(0);
	}
	pc->send_packet(&packet);
}

void ChannelManager::send_channel_full(pc* pc) {
	Packet packet;
	packet.write<uint16>(0x4e);
	packet.write<uint8>(2);
	pc->send_packet(&packet);
}

void ChannelManager::send_success(pc* pc) {
	Packet packet;
	packet.write<uint16>(0x4e);
	packet.write<uint8>(1);
	pc->send_packet(&packet);
}

/*************************************************************** END OF CHANNEL CLASS *******************************************************************/

void Channel::pc_enter_lobby(pc* pc) {
	sys_verify_pc(pc);

	if (pc->channel_in_) {
		return;
	}

	pc->channel_in_ = true;

	sys_send_pc_list(pc);
	sys_send_this_pc(pc, pc_send_type::pc_show_lobby);
	sys_send_enter_lobby(pc);
}

void Channel::pc_leave_lobby(pc* pc) {
	sys_verify_pc(pc);

	if (!pc->channel_) {
		return;
	}

	pc->channel_in_ = false;
	sys_send_this_pc(pc, pc_send_type::pc_leave_lobby);
	Channel::sys_send_leave_lobby(pc);
}

void Channel::pc_quit_lobby(pc* pc) {
	sys_verify_pc(pc);

	auto it = std::find(std::begin(pc_list), std::end(pc_list), pc);

	if (it == std::end(pc_list)) {
		return;
	}
	
	if (pc->channel_in_) {
		sys_send_this_pc(pc, pc_send_type::pc_leave_lobby);
	}

	pc_list.erase(it);
}

void Channel::pc_send_message(pc* pc) {
	struct item item;
	item.amount = 17;
	item.expire_day = 0;
	item.flag = 0;
	item.item_type = 2;
	item.type_id = 2092957703;


	std::shared_ptr<INV_CARD> card_out = std::make_shared<INV_CARD>();

	pc->inventory->addCard(pc, &item, false, false, &card_out);

	sys_verify_pc(pc);

	pc->read<uint32>();
	std::string pc_name = pc->read<std::string>();
	std::string pc_msg = pc->read<std::string>();

	if (pc->name_.compare(pc_name) != 0) {
		return;
	}

	if (!(pc->game_id == 0xFFFF) && (pc->channel_in_)) {
		sys_send_pc_message(pc, pc_msg);
		return;
	}
}

void Channel::sys_send_pc_list(pc* pc) {
	Packet packet;
	packet.write<uint16>(0x46);
	packet.write<uint8>(4); // show player

	int c = 0;

	for (auto pc : pc_list) {
		if (pc->channel_in_) {
			c += 1;
		}
	}

	packet.write<uint8>(c);

	for (auto pc : pc_list) {
		if (pc->channel_in_) {
			sys_get_pc_data(pc, &packet);
		}
	}
	pc->send_packet(&packet);
}

void Channel::sys_send_this_pc(pc* pc, enum pc_send_type a) {
	Packet packet;
	packet.write<uint16>(0x46);
	packet.write<uint8>(a);
	packet.write<uint8>(1); // this should always be 1
	sys_get_pc_data(pc, &packet);
	sys_send(packet);
}

void Channel::sys_get_pc_data(pc* pc, Packet* packet) {
	packet->write<uint32>(pc->account_id_);
	packet->write<uint32>(pc->connection_id_);
	packet->write<__int16>(pc->game_id);
	packet->write_string(pc->name_, 16);
	packet->write_null(6);
	packet->write<uint8>(35); // level
	packet->write<uint32>(0); // gm visible?
	packet->write<uint32>(0); // title typeid
	packet->write<uint32>(1000); // what?
	packet->write<uint8>(pc->sex_);
	packet->write<uint32>(0); // guild id
	packet->write_string("", 9); // guild image
	packet->write_null(3);
	packet->write<uint8>(1); // vip?
	packet->write_null(6);
	packet->write_string(pc->username_ + "@NT", 18);
	packet->write_null(0x6e);
}

void Channel::sys_send_pc_message(pc* pc, std::string& message) {
	Packet packet;
	packet.write<uint16>(0x40);
	packet.write<uint8>(pc->capability_ == 4 ? 0x80 : 0);
	packet.write<std::string>(pc->name_);
	packet.write<std::string>(message);
	sys_send(packet);
}

void Channel::sys_send_enter_lobby(pc* pc) {
	Packet packet;
	packet.write<uint16>(0xf5);
	pc->send_packet(&packet);
}

void Channel::sys_send_leave_lobby(pc* pc) {
	Packet packet;
	packet.write<uint16>(0xf6);
	pc->send_packet(&packet);
}

void Channel::sys_send(Packet& packet) {
	std::for_each(pc_list.begin(), pc_list.end(), [&packet](pc* pc) {
		pc->send_packet(&packet);
	});
}

void Channel::sys_verify_pc(pc* pc) {
	if (pc == nullptr)
		return throw ChannelNotFound();

	auto it = std::find(pc_list.begin(), pc_list.end(), pc);

	if (it == pc_list.end()) {
		return throw ChannelNotFound();
	}
}