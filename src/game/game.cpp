#include <iostream>
#include "../common/unique.h"
#include "../common/db.h"
#include "../common/crypto.h"
#include "../common/timer.h"

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
	delete itemdb;
	delete shop;
	delete timer;
}

class c {
public:
	c() { printf("class created!");  };
	~c() { printf("class destroyed"); };
};

int main(int argc, char *argv[]) {
	auto console = spdlog::stdout_color_mt("console");
	try {
#ifdef SIGBREAK
		signal(SIGBREAK, signal_handler);
#endif
		signal(SIGINT, signal_handler);
		signal(SIGTERM, signal_handler);

		// Initializing
		timer = new TimerQueue();
		unique_s = new unique_id(10000);
		sdb = new db();
		crypt = new Crypto();
		config = new Config();
		sclif = new clif();
		pc_process = new account();
		pcs = new pc_manager();
		channel_manager = new ChannelManager();
		itemdb = new ItemDB();
		shop = new ShopSystem();

		boost::asio::io_context io_context;
		Socket server(io_context, config->read->GetInteger("game", "port", 20201));
		io_context.run();
	}
	catch (std::exception& e) {
		std::cerr << "[ERROR]" << e.what() << std::endl;
		system("pause");
	}
}