#pragma once
#include "ServiceBase.h"
#include <cstdint>
#include <optional>
#include <string>

//A class for recieving one-way messages
class TCPReceiver : private serviceBase
{
	asio::ip::tcp::acceptor acceptor;
	asio::ip::tcp::socket socket;

	//A buffer containing the message so far
	std::string buffer;
	//Indicates where in the buffer the next message should write to
	uint32_t writePos = 0;
	//Maximum number of bytes to read in one go
	uint16_t blockSize = 1000;

	static constexpr auto sizeInfoSize = sizeof(uint32_t);

	//Whether or not a connection has been accepted
	bool accepted = false;
	//Whether the class has recieved size information for the current message
	bool sized = false;
	//Whether the current message has finished sending
	bool complete = false;

	void handle_read(const asio::error_code& ec, size_t bytes)
	{
		if (!ec)
		{
			if (!sized)
			{
				//If the size has not been recieved, assume it is the current packet
				if (bytes != sizeInfoSize)
					return;

				uint32_t messageSize =
					(buffer[3] << 24) |
					(buffer[2] << 16) |
					(buffer[1] << 8)  |
					(buffer[0] << 0);

				//Reset the buffer
				writePos = 0;
				buffer.resize(messageSize);
				sized = true;
				start_read();
				return;
			}

			writePos += bytes;
			if (writePos >= buffer.size())
			{
				complete = true;
			}
		}
		start_read();
	}

	void start_read()
	{
		//When a message is not yet recieved or the previous message has completed recieving, expect to receive a size next
		if (!sized || complete)
		{
			socket.async_receive(asio::buffer(buffer.data(), sizeInfoSize),
				std::bind(&TCPReceiver::handle_read, this,
					std::placeholders::_1, std::placeholders::_2));
		}
		else
		{
			socket.async_receive(asio::buffer(buffer.data() + writePos, blockSize),
				std::bind(&TCPReceiver::handle_read, this,
					std::placeholders::_1, std::placeholders::_2));
		}
	}


	void handle_accept(const asio::error_code& ec)
	{
		if (!ec)
		{
			//Once the connection has been confirmed, begin reading messages
			accepted = true;
			start_read();
		}
		else
		{
			//If the connection is rejected, start waiting for another one
			start_accept();
		}
	}

	void start_accept()
	{
		acceptor.async_accept(socket,
			std::bind(&TCPReceiver::handle_accept, this,
				std::placeholders::_1));
	}

public:
	TCPReceiver(unsigned short port, uint16_t readBlockSize = 1024) : acceptor(service, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
		socket(service), blockSize(readBlockSize)
	{
		buffer.resize(sizeInfoSize);
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
		buffer.resize(sizeInfoSize);
		sized = false;
		complete = false;
	}
};
