#pragma once
#include "ServiceBase.h"
#include "UDPMessage.h"
#include "UDPRequests.h"
#include <cstdint>
#include <optional>
#include <iostream>

class UDPReceiver : private serviceBase
{
	//The socket being listened on
	asio::ip::udp::socket socket;
	//The endpoint of the last broadcast received
	asio::ip::udp::endpoint remote;

	//Last received data
	UDPDataHandler data;

	//Called once the system has finished sending some data
	void handle_message(const asio::error_code&, size_t)
	{
		//Do nothing, we let the caller handle this
	}

	//Called once the system has finished recieving some data
	void handle_receive(const asio::error_code& ec, size_t bytes)
	{
		start_receive();
	}

	//Waits for data to be received before calling handle_receive
	void start_receive()
	{
		socket.async_receive_from(
			data.getBuffer(), remote,
			std::bind(&UDPReceiver::handle_receive, this,
				std::placeholders::_1, std::placeholders::_2));
	}

public:

	//Constructs a new responder bound to a given service, does not start listening immediately
	//Binds the responder to the given port
	UDPReceiver(unsigned short port, const std::string& address, const std::string& interfaceAddress = "") : socket(service)
	{
		//Create an endpoint for the multicast address and port
		asio::ip::udp::endpoint multicast_endpoint(asio::ip::address_v4::any(), port);

		//Open the socket and bind it to the multicast endpoint
		socket.open(asio::ip::udp::v4());
		socket.set_option(asio::ip::udp::socket::reuse_address(true));
		socket.bind(multicast_endpoint);

		//Join the multicast group
		if (interfaceAddress.empty())
		{
			asio::ip::multicast::join_group option(asio::ip::address::from_string(address));
			socket.set_option(option);
		}
		else
		{
			asio::ip::multicast::join_group option(asio::ip::address_v4::from_string(address), asio::ip::address_v4::from_string(interfaceAddress));
			socket.set_option(option);
		}

		start_receive();
	}

	//Awaits any request from a device, the data segment can be used to determine the request being provided
	std::optional<UDPMessage> awaitRequest(uint16_t timeoutMS)
	{
		if (service.run_one_for(std::chrono::milliseconds(timeoutMS)) != 0)
		{
			UDPMessage ret;
			ret.endpoint = remote;
			ret.data = data;
			return ret;
		}
		return {};
	}

	//Sends a response to a specific endpoint
	void respond(const UDPMessage& respondTo, UDPRequest response)
	{
		respond(respondTo.endpoint, response);
	}

	//Sends a response to a specific endpoint
	void respond(const asio::ip::udp::endpoint& respondTo, UDPRequest response)
	{
		UDPDataHandler buf(response);
		socket.send_to(buf.getBuffer(), respondTo);
	}
};
