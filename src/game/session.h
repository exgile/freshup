#pragma once

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include "../common/asio.hpp"

class pc;
class Packet;

using boost::asio::ip::tcp;

class Session : public boost::enable_shared_from_this<Session> {
	private:
		tcp::socket socket_;
		std::unique_ptr<unsigned char> receive_buffer_;
		pc* pc_;
		unsigned __int8 key_;

	public:
		typedef boost::shared_ptr<Session> pointer;

		// Default Constructor
		Session(boost::asio::io_context& io_context);

		// Default Destructor
		~Session();

		static pointer create(boost::asio::io_context& io_context);
		boost::asio::ip::tcp::socket& get_socket();
		unsigned char* get_receive_buffer();
		void send_packet_undecrypt(Packet *packet);
		void send_packet(Packet *packet);
		void send_handler(const boost::system::error_code &ec, std::size_t bytes_transferred, unsigned char *send_buffer);
		void send_handler_undecrypt(const boost::system::error_code &ec, std::size_t bytes_transferred) {};

		void read_header();
		void handle_read_header(const boost::system::error_code &ec, std::size_t bytes_transferred);
		void handle_read_body(const boost::system::error_code &ec, std::size_t bytes_transferred);
		void initialise();
		void disconnect();
};