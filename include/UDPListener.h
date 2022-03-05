#pragma once
#include <asio.hpp>
#include "UDPRequests.h"

class UDPResponder : private serviceBase
{
	//The socket being listened on
	asio::ip::udp::socket socket;
	//The endpoint of the last broadcase recieved
	asio::ip::udp::endpoint remote;
	//Last recieved data
	UDPDataHandler data;

	//Called once the system has finished sending some data
	void handle_message(const asio::error_code&, size_t)
	{
		//Do nothing, we let the caller handle this
	}

	//Called once the system has finished recieving some data
	void handle_recieve(const asio::error_code& ec, size_t bytes)
	{
		if (!ec)
		{
			UDPDataHandler buf(UDPRequest::respondAddress);
			socket.send_to(buf.getBuffer(), remote);
			start_recieve();
		}
	}

	//Waits for data to be recieved before calling handle_recieve
	void start_recieve()
	{
		socket.async_receive_from(
			data.getBuffer(), remote,
			std::bind(&UDPResponder::handle_recieve, this,
				std::placeholders::_1, std::placeholders::_2));
	}

public:

	//A combination of data and a sender
	struct response
	{
		asio::ip::udp::endpoint endpoint;
		UDPDataHandler data;
	};

	//Constructs a new responder bound to a given service, does not start listening immediately
	//Binds the responder to the given port
	UDPResponder(unsigned short port) : socket(service, asio::ip::udp::endpoint(asio::ip::udp::v4(), port))
	{
		start_recieve();
	}

	//Awaits any request from a device, the data segment can be used to determine the request being provided
	std::optional<response> awaitRequest(uint16_t timeoutMS)
	{
		if (service.run_one_for(std::chrono::milliseconds(timeoutMS)) != 0)
		{
			response ret;
			ret.endpoint = remote;
			ret.data = data;
			return ret;
		}
		return {};
	}

	//Sends a response to a specific endpoint
	void respond(const response& respondTo, UDPRequest response)
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
