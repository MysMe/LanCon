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

constexpr auto UDPPort = 40404, TCPPort = 40405;

void fullLinkOut()
{
	asio::io_service service;
	asio::ip::address confirmed;

	UDPBroadcaster broadcast(service, UDPPort);

	broadcast.setOnRespond(
		[&](const asio::ip::address& address, const UDPDataHandler data)
		{
			if (data.getRequest() == UDPRequest::respondAddress)
			{
				std::cout << "Recieved response from " << address.to_string() << ".\n";
				std::cout << "Requesting link...\n";
				broadcast.requestLink(address, TCPPort);
				return;
			}
			if (data.getRequest() == UDPRequest::acceptLink)
			{
				std::cout << address.to_string() << " accepted link request.\n";
				confirmed = address;
				service.stop();
			}
			if (data.getRequest() == UDPRequest::denyLink)
			{
				std::cout << address.to_string() << " denied link request, continuing scan...\n";
				return;
			}
			std::cout << "Unknown request recieved: " << static_cast<uint8_t>(data.getRequest()) << ".\n";
		}
	);

	std::cout << "Sending requests...\n";
	while (confirmed == asio::ip::address{})
	{
		broadcast.requestAddress();
		service.run_for(std::chrono::seconds(2));
	}

	std::cout << "Opening link...\n";

	TCPSender sender(confirmed.to_string(), std::to_string(TCPPort), 1000);
	while (!sender.connected())
	{
		std::cout << "Connection failed, retrying...";
		sender.connect(confirmed.to_string(), std::to_string(TCPPort), 1000);
	}

	std::cout << "Connection ready, begin typing.\n";

	while (true)
	{
		std::string line;
		std::getline(std::cin, line);
		sender.send(line);
	}
}

void fullLinkIn()
{
	asio::io_service service;
	UDPResponder responder(service, UDPPort);
	bool linked = false;

	responder.setOnHear(
		[&](const asio::ip::address& address, const UDPDataHandler& data)
		{
			if (linked)
			{
				std::cout << "Address requested by " << address.to_string() << " but a link has already been accepted. Ignoring.";
				return UDPDataHandler(UDPRequest::denyLink);
			}
			if (data.getRequest() == UDPRequest::requestAddress)
			{
				std::cout << "Address requested by " << address.to_string() << ".\n";
				return UDPDataHandler(UDPRequest::respondAddress);
			}
			if (data.getRequest() == UDPRequest::requestLink)
			{
				std::cout << "Link requested by " << address.to_string() << ".\n";
				std::cout << "Accept link? [Y/N]\n";
				char v;
				do
				{
					std::cin >> v;
				} while (v != 'Y' && v != 'N');
				if (v == 'Y')
				{
					std::cout << "Opening link...\n";
					service.stop();
					return UDPDataHandler(UDPRequest::acceptLink);
				}
				else
				{
					std::cout << "Denying link...\n";
					return UDPDataHandler(UDPRequest::denyLink);
				}
			}
			std::cout << "Unknown request recieved: " << static_cast<uint8_t>(data.getRequest()) << ".\n";
			return UDPDataHandler(UDPRequest::undefined);
		}
	);

	std::cout << "Waiting for requests...\n";

	service.run();

	TCPListener listener(service, TCPPort);

	std::cout << "Waiting for link...\n";

	service.run();
}

int main()
{
	fullLinkIn();
}