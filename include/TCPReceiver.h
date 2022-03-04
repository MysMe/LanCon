#pragma once
#include "ServiceBase.h"
#include <optional>

class TCPListener : private serviceBase
{
	asio::ip::tcp::acceptor acceptor;
	asio::ip::tcp::socket socket;

	std::string buffer;
	uint16_t blockSize = 1000;

	bool accepted = false;
	bool complete = false;

	void handle_read(const asio::error_code& ec, size_t bytes)
	{
		if (ec == asio::error::eof)
		{
			complete = true;
			return;
		}
		if (!ec)
		{
			buffer.resize(buffer.size() + bytes - blockSize);
			buffer.clear();
		}
		start_read();
	}

	void start_read()
	{
		std::size_t start = buffer.size();
		buffer.resize(buffer.size() + blockSize, '\0');
		socket.async_receive(asio::buffer(buffer.data() + start, blockSize),
			std::bind(&TCPListener::handle_read, this,
				std::placeholders::_1, std::placeholders::_2));
	}


	void handle_accept(const asio::error_code& ec)
	{
		if (!ec)
		{
			accepted = true;
		}
	}

	void start_accept()
	{
		acceptor.async_accept(socket,
			std::bind(&TCPListener::handle_accept, this,
				std::placeholders::_1));
	}

public:
	TCPListener(unsigned short port, uint16_t readBlockSize = 1000) : acceptor(service, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
		socket(service), blockSize(readBlockSize)
	{
		start_accept();
	}

	std::optional<asio::ip::address> awaitAccept(uint16_t timeoutMS)
	{
		if (accepted)
			return {};

		service.run_one_for(std::chrono::milliseconds(timeoutMS));

		if (accepted)
		{
			return socket.remote_endpoint().address();
		}
		return {};
	}

	std::optional<std::reference_wrapper<const std::string>> awaitMessage(uint16_t subTimeoutMS)
	{
		while (!complete)
		{
			if (service.run_one_for(std::chrono::milliseconds(subTimeoutMS)) == 0)
			{
				return {};
			}
		}
		return std::cref(buffer);
	}

	void clearBuffer()
	{
		if (!accepted || !complete)
			return;

		buffer.clear();
		complete = false;
	}
};
