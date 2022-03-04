#pragma once
#include "ServiceBase.h"
#include <optional>

class TCPListener : private serviceBase
{
	asio::ip::tcp::acceptor acceptor;
	asio::ip::tcp::socket socket;

	std::string buffer;
	uint32_t writePos = 0;
	uint16_t blockSize = 1000;

	bool accepted = false;
	bool sized = false;
	bool complete = false;

	void handle_read(const asio::error_code& ec, size_t bytes)
	{
		if (!ec)
		{
			if (!sized)
			{
				if (bytes != 4)
					return;

				uint32_t messageSize =
					(buffer[3] << 24) |
					(buffer[2] << 16) |
					(buffer[1] << 8)  |
					(buffer[0] << 0);

				std::cout << "Message size: " << messageSize << ".\n";
				buffer.clear();
				buffer.resize(messageSize);
				sized = true;
				start_read();
				return;
			}

			writePos += bytes;
			if (writePos >= buffer.size())
			{
				complete = true;
				return;
			}
		}
		start_read();
	}

	void start_read()
	{
		socket.async_receive(asio::buffer(buffer.data() + writePos, blockSize),
			std::bind(&TCPListener::handle_read, this,
				std::placeholders::_1, std::placeholders::_2));
	}


	void handle_accept(const asio::error_code& ec)
	{
		if (!ec)
		{
			accepted = true;
			start_read();
		}
		else
		{
			start_accept();
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
		if (blockSize < 10)
			blockSize = 10;
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

		writePos = 0;
		buffer.clear();
		sized = false;
		complete = false;
		start_read();
	}
};
