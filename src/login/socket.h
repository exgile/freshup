#pragma once

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include "session.h"

using boost::asio::ip::tcp;

class Socket {
	private:
		tcp::acceptor acceptor_;
		void start_accept();
		void handle_accept(Session::pointer pc_session, const boost::system::error_code& error);

	public:
		Socket(boost::asio::io_context& io_context, int port);
};
