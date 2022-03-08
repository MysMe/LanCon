#pragma once
#include "ServiceBase.h"
#include <cstdint>
#include <string>

//A class for one-way sending of messages
class TCPSender : private serviceBase
{
	asio::ip::tcp::resolver resolver;
	asio::ip::tcp::socket socket;
	//Whether or not the socket has been connected to anything
	bool isConnected = false;

	void handle_connect(const asio::error_code& ec)
	{
		//Only connect if this event is called without error
		if (!ec)
			isConnected = true;
	}

public:

	TCPSender()
		: socket(service), resolver(service)
	{}

	//Construct and connect in one function
	TCPSender(const std::string& IP, const std::string& port, std::size_t timeoutMS)
		: socket(service), resolver(service)
	{
		connect(IP, port, timeoutMS);
	}

	~TCPSender()
	{
		disconnect();
	}

	void disconnect()
	{
		if (connected())
		{
			//Gracefully shuts down both ends of the connection
			socket.shutdown(socket.shutdown_both);
			isConnected = false;
		}
	}

	bool connect(const std::string& IP, const std::string& port, std::size_t timeoutMS)
	{
		disconnect();

		auto endpoint = *resolver.resolve(
			asio::ip::tcp::resolver::query(asio::ip::tcp::v4(), IP, port));

		socket.async_connect(endpoint,
			std::bind(&TCPSender::handle_connect, this, std::placeholders::_1));

		service.run_one_for(std::chrono::milliseconds(timeoutMS));
		service.stop();
		service.reset();

		return connected();
	}

	bool connected() const
	{
		return isConnected;
	}

	void send(const std::string& message)
	{
		//Do not send empty messages, asio does not send empty TCP packets, intefering with the size-based messaging system
		if (message.empty())
			return;

		const uint32_t size = message.size();
		//Ensure the data is consistent between various endian systems
		std::array<uint8_t, 4> bytes;
		bytes[0] = (size >> 0) & 0xFF;
		bytes[1] = (size >> 8) & 0xFF;
		bytes[2] = (size >> 16) & 0xFF;
		bytes[3] = (size >> 24) & 0xFF;

		socket.send(asio::buffer(bytes));
		socket.send(asio::buffer(message));
	}
};