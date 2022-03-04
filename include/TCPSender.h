#pragma once
#include "ServiceBase.h"

class TCPSender : private serviceBase
{
	asio::ip::tcp::resolver resolver;
	asio::ip::tcp::socket socket;
	bool isConnected = false;

	void handle_connect(const asio::error_code& ec)
	{
		if (!ec)
			isConnected = true;
	}

public:

	TCPSender()
		: socket(service), resolver(service)
	{}

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
		const uint32_t size = message.size();
		//Ensure the data is consistent between various endian systems
		uint8_t lolo = (size >> 0) & 0xFF;
		uint8_t lohi = (size >> 8) & 0xFF;
		uint8_t hilo = (size >> 16) & 0xFF;
		uint8_t hihi = (size >> 24) & 0xFF;

		socket.send(asio::buffer(&lolo, 4));
		socket.send(asio::buffer(message));
	}
};