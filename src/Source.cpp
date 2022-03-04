#include <iostream>
#include "UDPBroadcaster.h"
#include "UDPListener.h"
#include "TCPSender.h"
#include "TCPReceiver.h"



//Asynchronous

constexpr auto UDPPort = 40404, TCPPort = 40405;

void fullLinkOut()
{
	asio::ip::address confirmed;

	UDPBroadcaster broadcast(UDPPort);

	broadcast.requestAddress();
	while (true)
	{
		auto v = broadcast.awaitResponse(1000);
		if (!v)
		{
			broadcast.requestAddress();
		}
		else
		{
			if (v.value().data.getRequest() != UDPRequest::respondAddress)
				continue;

			std::cout << "Response recieved from " << v.value().endpoint.address().to_string() << ".\n";
			std::cout << "Attempt link? [Y/N]\n";
			char c;
			do
			{
				std::cin >> c;
			} while (c != 'Y' && c != 'N');
			if (c == 'Y')
			{
				confirmed = v.value().endpoint.address();
				break;
			}
		}
	}

	std::cout << "Sending link request...\n";
	while (true)
	{
		broadcast.requestLink(confirmed, TCPPort);
		auto v = broadcast.awaitResponse(1000);
		if (v)
		{
			if (v.value().data.getRequest() == UDPRequest::denyLink)
			{
				std::cout << "Link denied. Closing...";
				std::cin.ignore();
				return;
			}
			if (v.value().data.getRequest() == UDPRequest::acceptLink)
			{
				std::cout << "Link accepted.\n";
				break;
			}
		}
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
	UDPResponder responder(UDPPort);
	asio::ip::udp::endpoint confirmed;
	std::cout << "Waiting for requests...\n";

	while (true)
	{
		auto v = responder.awaitRequest(1000);
		if (!v)
			continue;
		if (v.value().data.getRequest() == UDPRequest::requestAddress)
		{
			std::cout << ".";
			responder.respond(v.value(), UDPRequest::respondAddress);
		}
		else if (v.value().data.getRequest() == UDPRequest::requestLink)
		{
			std::cout << "\nLink request recieved from " << v.value().endpoint.address().to_string() << "\nAccept? [Y/N]\n";
			char c;
			do
			{
				std::cin >> c;
			} while (c != 'Y' && c != 'N');
			if (c == 'Y')
			{
				confirmed = v.value().endpoint;
				break;
			}
		}
	}
	
	std::cout << "Awaiting link.\n";

	TCPListener listener(TCPPort);

	while (true)
	{
		auto v = listener.awaitAccept(1000);
		if (v)
		{
			std::cout << "\nLink accepted.\n";
			break;
		}
		else
		{
			responder.respond(confirmed, UDPRequest::acceptLink);
			std::cout << ".";
		}
	}

	while (true)
	{
		auto v = listener.awaitMessage(1000);
		if (v)
		{
			std::cout << "\n" << v.value().get() << "\n";
			listener.clearBuffer();
		}
		else
		{
			std::cout << ".";
		}
	}
}

int main()
{
	fullLinkIn();
}