#include <iostream>
#include "../common/db.h"
#include "../common/crypto.h"
#include "../common/timer.h"
#include "../common/utils.h"
#include "../common/timer.h"

#include "spdlog/sinks/stdout_color_sinks.h"
#include "socket.h"
#include "pc_manager.h"
#include "reader.h"
#include "channel.h"
#include "itemdb.h"
#include "packetdb.h"

#include <boost/format.hpp>

void signal_handler(int sig) {
	pcm->kickall();
	delete chm;
	delete pcm;
	delete itemdb;
	config_final();
	crypt_final();
	db_final();
	timer_final();
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
		db_init();
		crypt_init();
		packetdb_init();
		rnd_init();
		config_init();
		timer_init();
		pcm = new PC_Manager();
		chm = new ChannelManager();
		itemdb = new ItemDB();

		boost::asio::io_context io_context;
		Socket server(io_context, config->GetInteger("game", "port", 20201));

		while (true) {
			io_context.poll();
			do_timer();
			std::this_thread::sleep_for(std::chrono::microseconds(10)); // Prevent CPU cycle loop
		}
	}
	catch (std::exception& e) {
		std::cerr << "[ERROR]" << e.what() << std::endl;
		system("pause");
	}
}