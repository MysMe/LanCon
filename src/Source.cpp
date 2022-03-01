#include <iostream>
#include <asio.hpp>
#include <optional>


class UDPScanner
{
	//No data is ever sent, so a "null" buffer is used in most cases
	static auto nullBuffer()
	{
		return asio::buffer((char*)nullptr, 0);
	}

public:

	//Sends a broadcast and returns the address of the first responder
	std::optional<asio::ip::address> ping(unsigned short port)
	{
		using asio::ip::udp;

		//Initialise socket
		asio::io_service service;
		udp::socket socket(service);

		//Open socket on port as UDP
		asio::error_code ec;
		socket.open(udp::v4(), ec);
		if (ec)
			return {};

		//Set socket for broadcast
		socket.set_option(asio::ip::udp::socket::reuse_address(true));
		socket.set_option(asio::socket_base::broadcast(true));
		auto endpoint = udp::endpoint(asio::ip::address_v4::broadcast(), port);

		//Send broadcast
		socket.send_to(nullBuffer(), endpoint);

		udp::endpoint response;
		std::size_t length = socket.receive_from(
			nullBuffer(), response);

		if (length != 0)
			return {};

		socket.close();
		return response.address();
	}

	std::optional<asio::ip::address> listen(unsigned short port)
	{
		using asio::ip::udp;
		asio::io_service service;
		udp::socket socket(service, udp::endpoint(udp::v4(), port));


		udp::endpoint remote;
		asio::error_code ec;

		std::size_t contentSize = socket.receive_from(
			nullBuffer(), remote, 0, ec);

		if (ec)
		{
			return {};
		}

		asio::error_code ignore;
		socket.send_to(nullBuffer(), remote, 0, ignore);
		socket.close();
		return remote.address();
	}
};

void autoSpeak()
{
	UDPScanner host;
	std::cout << "Scanning...\n";
	std::cout << host.ping(40404).value();
	std::cin.ignore();
}

void autoListen()
{
	UDPScanner client;
	std::cout << "Waiting...\n";
	std::cout << client.listen(40404).value();
	std::cin.ignore();
}

int main()
{
#ifndef _DEBUG
	autoSpeak();
#else
	autoListen();
#endif // DEBUG

}