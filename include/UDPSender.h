#pragma once
#include "ServiceBase.h"
#include "UDPMessage.h"
#include "UDPRequests.h"
#include <cstdint>
#include <optional>

//A class designed to send broadcast packets in order to discover other devices running this program
class UDPSender : private serviceBase
{
	//The last endpoint to respond to a query
	asio::ip::udp::endpoint responder;
	//The socket being used
	asio::ip::udp::socket socket;
	//The port being broadcast/received on
	unsigned short port;
	asio::ip::address mcad;
	//Most recently received data packet
	UDPDataHandler data;

	//Called once the socket has received some information
	void handle_receive(const asio::error_code& ec, size_t bytes)
	{
		//Enqueue another receive
		wait_receive();
	}

	//Enqueues a receive event but does not directly block waiting for it
	void wait_receive()
	{
		socket.async_receive_from(
			data.getBuffer(), responder,
			std::bind(&UDPSender::handle_receive, this,
				std::placeholders::_1, std::placeholders::_2));

	}
public:

	//Constructs a new broadcaster bound to the given service, does not in itself start broadcasting
	UDPSender(unsigned short port, const std::string& address) : port(port), socket(service)
	{
		//Manually open the socket and enable broadcasting
		socket.open(asio::ip::udp::v4());
		socket.set_option(asio::ip::udp::socket::reuse_address(true));

		asio::ip::address multicastIp = asio::ip::address::from_string(address);
		asio::ip::multicast::join_group option(multicastIp);
		socket.set_option(option);
		mcad = multicastIp;

		wait_receive();
	}

	//Sends a broadcast searching for any devices running this program
	void requestAddress()
	{
		UDPDataHandler send(UDPRequest::requestAddress);
		socket.send_to(send.getBuffer(), asio::ip::udp::endpoint(mcad, port));
	}

	//Sends a request to a specific device asking to open a TCP connection
	void requestLink(const asio::ip::address& target, uint16_t linkPort)
	{
		UDPDataHandler send(UDPRequest::requestLink, linkPort);
		socket.send_to(send.getBuffer(), asio::ip::udp::endpoint(target, port));
	}

	//Waits for a response to any request, the data segment indicates which request is being responded to
	std::optional<UDPMessage> awaitResponse(uint16_t timeoutMS)
	{
		const auto b = service.stopped();
		//The only event that can run is a receive, ergo either 1 receive runs or none run
		if (service.run_one_for(std::chrono::milliseconds(timeoutMS)) != 0)
		{
			//Ignore default address as this binds to anything
			if (responder.address().is_unspecified())
				return {};

			UDPMessage ret;
			ret.data = data;
			ret.endpoint = responder;
			return ret;
		}
		return {};
	}
};
