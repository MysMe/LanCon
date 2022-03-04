#pragma once
#include <asio.hpp>
#include "UDPRequests.h"
#include "ServiceBase.h"
#include <optional>

class UDPBroadcaster : private serviceBase
{
	//The last endpoint to respond to a query
	asio::ip::udp::endpoint responder;
	//The socket being used
	asio::ip::udp::socket socket;
	//The port being broadcast/recieved on
	unsigned short port;

	UDPDataHandler data;

	//Called once the socket has recieved some information
	void handle_recieve(const asio::error_code& ec, size_t bytes)
	{
	}

	//Called once the socket has completely sent some information
	void wait_recieve()
	{
		//Wait to recieve some information
		socket.async_receive_from(
			data.getBuffer(), responder,
			std::bind(&UDPBroadcaster::handle_recieve, this,
				std::placeholders::_1, std::placeholders::_2));

	}
public:

	struct response
	{
		asio::ip::udp::endpoint endpoint;
		UDPDataHandler data;
	};

	//Constructs a new broadcaster bound to the given service, does not in itself start broadcasting
	UDPBroadcaster(unsigned short port) : port(port), socket(service)
	{
		//Manually open the socket and enable broadcasting
		socket.open(asio::ip::udp::v4());
		socket.set_option(asio::ip::udp::socket::reuse_address(true));
		socket.set_option(asio::socket_base::broadcast(true));
	}

	void requestAddress()
	{
		UDPDataHandler send(UDPRequest::requestAddress);
		socket.send_to(send.getBuffer(), asio::ip::udp::endpoint(asio::ip::address_v4::broadcast(), port));
		wait_recieve();
	}

	void requestLink(const asio::ip::address& target, uint16_t linkPort)
	{
		UDPDataHandler send(UDPRequest::requestLink, linkPort);
		socket.send_to(send.getBuffer(), asio::ip::udp::endpoint(target, port));
		wait_recieve();
	}

	std::optional<response> awaitResponse(uint16_t timeoutMS)
	{
		if (service.run_one_for(std::chrono::milliseconds(timeoutMS)) != 0)
		{
			response ret;
			ret.data = data;
			ret.endpoint = responder;
			return ret;
		}
		return {};
	}
};
