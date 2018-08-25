#include <iostream>
#include "../common/unique.h"
#include "../common/db.h"
#include "../common/crypto.h"
#include "../common/queue.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "socket.h"
#include "clif.h"
#include "pc_manager.h"
#include "reader.h"
#include "account.h"
#include "channel.h"
#include "itemdb.h"
#include "shop.h"

void signal_handler(int sig) {
	delete unique_s;
	delete sdb;
	delete crypt;
	delete config;
	delete sclif;
	delete pc_process;
	delete pcs;
	delete channel_manager;
	delete queue;
	delete itemdb;
	delete shop;

	//system("pause");
}

int main(int argc, char *argv[]) {
	auto console = spdlog::stdout_color_mt("console");
	try {
#ifdef SIGBREAK
		signal(SIGBREAK, signal_handler);
#endif
		signal(SIGINT, signal_handler);
		signal(SIGTERM, signal_handler);

		// Initializing
		unique_s = new unique_id();
		sdb = new db();
		crypt = new Crypto();
		config = new Config();
		sclif = new clif();
		pc_process = new account();
		pcs = new pc_manager();
		channel_manager = new ChannelManager();
		queue = new Queue();
		itemdb = new ItemDB();
		shop = new ShopSystem();

		boost::asio::io_context io_context;
		Socket server(io_context, config->read->GetInteger("server", "port", 10201));
		io_context.run();
	}
	catch (std::exception& e) {
		std::cerr << "[ERROR]" << e.what() << std::endl;
		system("pause");
	}
}