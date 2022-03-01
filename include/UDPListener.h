#pragma once
#include <asio.hpp>
#include <set>

class UDPResponder
{
	//The socket being listened on
	asio::ip::udp::socket socket;
	//The endpoint of the last broadcase recieved
	asio::ip::udp::endpoint remote;
	//A list of addresses that have been recieved
	std::set<asio::ip::address> foundAddresses;

	//Called once the system has finished sending some data
	void handle_send(const asio::error_code&, size_t)
	{
		foundAddresses.insert(remote.address());
	}

	//Called once the system has finished recieving some data
	void handle_recieve(const asio::error_code& ec, size_t bytes)
	{
		if (!ec || ec == asio::error::message_size)
		{
			//Respond with a single byte
			socket.async_send_to(
				asio::buffer("\0", 1), remote,
				std::bind(&UDPResponder::handle_send, this,
					std::placeholders::_1, std::placeholders::_2));
			//Enque another wait to recieve
			start_recieve();
		}
	}

	//Waits for data to be recieved before calling handle_recieve
	void start_recieve()
	{
		std::array<char, 1> buf;
		socket.async_receive_from(
			asio::buffer(buf), remote,
			std::bind(&UDPResponder::handle_recieve, this,
				std::placeholders::_1, std::placeholders::_2));
	}

public:

	//Constructs a new responder bound to a given service, does not start listening immediately
	//Binds the responder to the given port
	UDPResponder(asio::io_service& service, unsigned short port) :
		socket(service, asio::ip::udp::endpoint(asio::ip::udp::v4(), port))
	{
		start_recieve();
	}

	const std::set<asio::ip::address> getAddresses() const
	{
		return foundAddresses;
	}

	void clearAddresses()
	{
		foundAddresses.clear();
	}
};
