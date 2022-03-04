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
		socket.send(asio::buffer(&size, sizeof(size)));
		socket.send(asio::buffer(message));
	}
};