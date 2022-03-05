#pragma once
#include <asio.hpp>
#include "UDPRequests.h"
#include "ServiceBase.h"
#include <optional>

//A class designed to send broadcast packets in order to discover other devices running this program
class UDPBroadcaster : private serviceBase
{
	//The last endpoint to respond to a query
	asio::ip::udp::endpoint responder;
	//The socket being used
	asio::ip::udp::socket socket;
	//The port being broadcast/recieved on
	unsigned short port;
	//Most recently recieved data packet
	UDPDataHandler data;

	//Called once the socket has recieved some information
	void handle_recieve(const asio::error_code& ec, size_t bytes)
	{
		//Do nothing, we let the caller handle this
	}

	//Enques a recieve event but does not directly block waiting for it
	void wait_recieve()
	{
		socket.async_receive_from(
			data.getBuffer(), responder,
			std::bind(&UDPBroadcaster::handle_recieve, this,
				std::placeholders::_1, std::placeholders::_2));

	}
public:

	//A combination of data and a sender
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

	//Sends a broadcast searching for any devices running this program
	void requestAddress()
	{
		UDPDataHandler send(UDPRequest::requestAddress);
		socket.send_to(send.getBuffer(), asio::ip::udp::endpoint(asio::ip::address_v4::broadcast(), port));
		wait_recieve();
	}

	//Sends a request to a specific device asking to open a TCP connection
	void requestLink(const asio::ip::address& target, uint16_t linkPort)
	{
		UDPDataHandler send(UDPRequest::requestLink, linkPort);
		socket.send_to(send.getBuffer(), asio::ip::udp::endpoint(target, port));
		wait_recieve();
	}

	//Waits for a response to any request, the data segment indicates which request is being responded to
	std::optional<response> awaitResponse(uint16_t timeoutMS)
	{
		//The only event that can run is a recieve, ergo either 1 receive runs or none run
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
