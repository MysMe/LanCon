#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::udp;

class serviceBase
{
protected:
	boost::asio::io_service service;

public:
	virtual ~serviceBase() = default;
};

class speaker : private serviceBase
{
	udp::resolver resolver;

public:

	speaker() : resolver(service)
	{
	}


	std::string request(const std::string& target, const std::string& message, const std::string& port)
	{
		std::string data;
		data.resize(1024);
		udp::resolver::query query(udp::v4(), target, port);
		auto endpoint = *resolver.resolve(query);

		udp::socket socket(service);
		socket.open(udp::v4());

		socket.send_to(boost::asio::buffer(message), endpoint);

		udp::endpoint response;
		std::size_t length = socket.receive_from(
			boost::asio::buffer(data.data(), data.size()), response);

		data.resize(length);
		return data;
	}
};

class listener : private serviceBase
{
	udp::socket socket;

public:

	listener(uint32_t port) : socket(service, udp::endpoint(udp::v4(), port))
	{

	}

	void listen()
	{
		std::string recieved;
		recieved.resize(10);

		udp::endpoint remote;

		boost::system::error_code ec;
		std::size_t contentSize = socket.receive_from(
		boost::asio::buffer(recieved.data(), recieved.size()),
			remote, 0, ec);


		if (ec)
		{
			std::cout << "Invalid request.\n";
			return;
		}

		recieved.resize(contentSize);

		std::cout << "Recieved: " << recieved << ".\n";

		std::string response = "1514131211109876543210";

		boost::system::error_code ignore;
		socket.send_to(boost::asio::buffer(response.data(), response.size()),
			remote, 0, ignore);
	}
};

void autoSpeak()
{
	speaker speak;
	std::cout << "Sending request:\nRecieved: ";
	std::cout << speak.request("localhost", "0123456789101112131415", "40404");
	std::cin.ignore();
}

void autoListen()
{
	listener listen(40404);
	std::cout << "Listening on 40404\n";
	listen.listen();
	std::cin.ignore();
}

int main()
{
#ifdef _DEBUG
	autoSpeak();
#else
	autoListen();
#endif // DEBUG

}