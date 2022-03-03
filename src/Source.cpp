#include <iostream>
#include <asio.hpp>
#include <set>
#include "UDPBroadcaster.h"
#include "UDPListener.h"

class TCPSenderService
{
protected:
	asio::io_service service;

public:
	virtual ~TCPSenderService() = default;
};

class TCPSender : private TCPSenderService
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
		socket.send(asio::buffer(message));
	}
};

//Asynchronous
class TCPListener
{
	asio::ip::tcp::acceptor acceptor;
	asio::ip::tcp::socket socket;
	std::string contents;
	std::string buffer;

	void handle_read(const asio::error_code& ec, size_t bytes)
	{
		if (!ec)
		{
			buffer.resize(bytes);
			std::cout << buffer << '\n';
			buffer.clear();
		}
		start_read();
	}

	void start_read()
	{
		buffer.resize(100, '\0');
		socket.async_receive(asio::buffer(buffer),
			std::bind(&TCPListener::handle_read, this,
				std::placeholders::_1, std::placeholders::_2));
	}

	void handle_accept(const asio::error_code& ec)
	{
		if (!ec)
		{
			std::cout << "Someone connected.\n";
			start_read();
		}
	}

	void start_accept()
	{
		acceptor.async_accept(socket,
			std::bind(&TCPListener::handle_accept, this,
				std::placeholders::_1));
	}

public:
	TCPListener(asio::io_service& service, unsigned short port)
		: acceptor(service, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
		socket(service)
	{
		start_accept();
	}
};


void autoSend()
{
	TCPSender send("localhost", "40404", 1000);
	while (!send.connected())
	{
		std::cout << "Failed to connect, retrying...\n";
		send.connect("localhost", "40404", 1000);
	}
	std::cout << "Connected.\n";
	while (true)
	{
		std::string line;
		std::getline(std::cin, line);
		send.send(line);
	}
}

void autoRecieve()
{
	asio::io_service service;
	TCPListener listen(service, 40404);
	service.run();
}

void autoListen()
{
	asio::io_service service;
	UDPResponder responder(service, 40404);

	std::cout << "Listening.\n";
	size_t addC = 0;
	while (true)
	{
		service.run_one_for(std::chrono::seconds(2));
		if (responder.getAddresses().size() != addC)
		{
			std::cout << "New address(es) found:\n";
			for (const auto& i : responder.getAddresses())
				std::cout << "\t" << i.to_string() << "\n";
			addC = responder.getAddresses().size();
		}
	}
}

void autoBroadcast()
{
	asio::io_service service;
	UDPBroadcaster broadcast(service, 40404);
	size_t addC = 0;
	while (true)
	{
		service.run_one_for(std::chrono::seconds(2));
		if (broadcast.getAddresses().size() != addC)
		{
			std::cout << "New address(es) found:\n";
			for (const auto& i : broadcast.getAddresses())
				std::cout << "\t" << i.to_string() << "\n";
			addC = broadcast.getAddresses().size();
		}
	}
}

int main()
{
	autoListen();
}