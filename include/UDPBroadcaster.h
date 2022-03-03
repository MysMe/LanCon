#pragma once
#include <asio.hpp>
#include <set>

class UDPBroadcaster
{
	//The last endpoint to respond to a query
	asio::ip::udp::endpoint responder;
	//The socket being used
	asio::ip::udp::socket socket;
	//The port being broadcast/recieved on
	unsigned short port;
	//A set of found addresses
	std::set<asio::ip::address> foundAddresses;

	//Called once the socket has recieved some information
	void handle_recieve(const asio::error_code& ec, size_t bytes)
	{
		foundAddresses.insert(responder.address());
	}

	//Called once the socket has completely sent some information
	void handle_send(const asio::error_code& ec, size_t)
	{
		if (!ec)
		{
			//Wait to recieve some information
			std::array<char, 1> buf{ 0 };
			socket.async_receive_from(
				asio::buffer(buf), responder,
				std::bind(&UDPBroadcaster::handle_recieve, this,
					std::placeholders::_1, std::placeholders::_2));

			//Enque another send
			start_send();
		}
	}

	//Broadcasts a single byte over the given port
	void start_send()
	{
		socket.async_send_to(asio::buffer("\0", 1),
			asio::ip::udp::endpoint(asio::ip::address_v4::from_string("239.255.0.1"), port),
			std::bind(&UDPBroadcaster::handle_send, this,
				std::placeholders::_1, std::placeholders::_2));
	}

public:

	//Constructs a new broadcaster bound to the given service, does not in itself start broadcasting
	UDPBroadcaster(asio::io_service& service, unsigned short port) : port(port),
		socket(service)
	{
		//Manually open the socket and enable broadcasting
		socket.open(asio::ip::udp::v4());
		socket.set_option(asio::ip::udp::socket::reuse_address(true));
		socket.set_option(asio::socket_base::broadcast(true));

		//Enque a broadcast
		start_send();
	}

	const std::set<asio::ip::address>& getAddresses() const
	{
		return foundAddresses;
	}

	void clearAddresses()
	{
		foundAddresses.clear();
	}
};
