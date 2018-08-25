#include "socket.h"
#include "spdlog/spdlog.h"

Socket::Socket(boost::asio::io_context& io_context, int port) : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
	spdlog::get("console")->info("Server is started accepting at port {}", port);
	start_accept();
}

void Socket::start_accept() {
	// create a new pc session and map the acceptor to pc session
	Session::pointer pc_session = Session::create(acceptor_.get_executor().context());

	// start async read and keep continue reading
	auto handler = boost::bind(&Socket::handle_accept, this, pc_session, boost::asio::placeholders::error);
	acceptor_.async_accept(pc_session->get_socket(), handler);
}

void Socket::handle_accept(Session::pointer pc_session, const boost::system::error_code& error) {
	if (!error) {
		// Start initialize the pc
		pc_session->initialise();
	}

	// keep accepting connections
	start_accept();
}