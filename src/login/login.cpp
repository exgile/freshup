#include <iostream>
#include "spdlog/sinks/stdout_color_sinks.h"
#include "clif.h"
#include "pcs.h"
#include "reader.h"
#include "account.h"
#include "socket.h"

#include "../common/db.h"
#include "../common/crypto.h"
#include "../common/utils.h"

void signal_handler(int sig) {
	crypt_final();
	delete config;
	delete sclif;
	delete pc_manager;
	db_final();
}

int main(int argc, char *argv[]) {

	try {
#ifdef SIGBREAK
		signal(SIGBREAK, signal_handler);
#endif
		signal(SIGINT, signal_handler);
		signal(SIGTERM, signal_handler);

		// Initializing
		rnd_init();
		db_init();
		crypt_init();
		config = new Config();
		sclif = new clif();
		pc_manager = new PC_Manager();

		auto console = spdlog::stdout_color_mt("console");

		boost::asio::io_context io_context;
		Socket server(io_context, config->read->GetInteger("login", "port", 10201));
		io_context.run();
	}
	catch (std::exception& e) {
		std::cerr << "[ERROR]" << e.what() << std::endl;
		system("pause");
	}
}