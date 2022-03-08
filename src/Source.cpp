#include "Interfaces.h"
#include "TCPSender.h"
#include "TCPReceiver.h"
#include "UDPSender.h"
#include "UDPReceiver.h"
#include <charconv>
#include <fstream>
#include <iostream>
#include <set>

struct CMDOptions
{
	enum class mode
	{
		none, //Invalid value
		listen, //Responds to both search and link requests
		broadcast, //Sends only search requests
		send //Sends a link request followed by a message
	};

	mode progMode = mode::none;

	//Default ports
	std::size_t UDPPort = 40404;
	std::size_t TCPPort = 40405;

	uint16_t frequency = 1000;

	asio::ip::address_v4 address;

	enum class contents
	{
		none, //No contents specified
		file, //Read or write to a file
		message //Read from a string
	};

	std::string data;

	contents contentType = contents::none;
};

void printOptions()
{
	std::cout <<
		"LanCon options:\n" <<
		"-l              | Enables listening mode.\n" <<
		"-b              | Enables broadcast mode. A broadcast is sent each time a request times out.\n" <<
		"-s    [ADDRESS] | Enables send mode targeting a specific device.\n" <<
		"-TCPPort [PORT] | Specifies which port to use for data transmission (-s only) (default: " << CMDOptions().TCPPort << ").\n" <<
		"-UDPPort [PORT] | Specifies which port to use for program discovery (all modes) (default: " << CMDOptions().UDPPort << ").\n" <<
		"-m    [MESSAGE] | Specifies a message to send to the end device.\n" <<
		"-f       [FILE] | Specifies a file to be written into or read from (-l/-s).\n" <<
		"-freq [MILSECS] | Specifies the delay between operations (all modes) (default: " << CMDOptions().frequency << ").\n\n";
}

std::optional<CMDOptions> parse(int argc, char** argv)
{
	CMDOptions ret;
	for (int i = 1; i < argc; i++)
	{
		const bool hasNext = i < (argc - 1);
		if (std::strcmp(argv[i], "-l") == 0)
		{
			if (ret.progMode != CMDOptions::mode::none)
			{
				std::cout << "Error: Multiple run modes specified.\n";
				return {};
			}
			ret.progMode = CMDOptions::mode::listen;
			continue;
		}

		if (std::strcmp(argv[i], "-b") == 0)
		{
			if (ret.progMode != CMDOptions::mode::none)
			{
				std::cout << "Error: Multiple run modes specified.\n";
				return {};
			}
			ret.progMode = CMDOptions::mode::broadcast;
			continue;
		}

		if (std::strcmp(argv[i], "-s") == 0)
		{
			if (ret.progMode != CMDOptions::mode::none)
			{
				std::cout << "Error: Multiple run modes specified.\n";
				return {};
			}
			ret.progMode = CMDOptions::mode::send;

			if (!hasNext)
			{
				std::cout << "Error: -s not followed by a value.\n";
				return {};
			}

			asio::error_code ec;
			ret.address = asio::ip::address_v4::from_string(argv[i + 1], ec);
			if (ec)
			{
				std::cout << "Error: Unable to parse address \"" << argv[i + 1] << "\".\n";
				return {};
			}

			i++;

			continue;
		}


		if (std::strcmp(argv[i], "-TCPPort") == 0)
		{
			if (!hasNext)
			{
				std::cout << "Error: -TCPPort not followed by a value.\n";
				return {};
			}

			auto [p, ec] = std::from_chars(argv[i + 1], argv[i + 1] + std::strlen(argv[i + 1]), ret.TCPPort);
			if (ec != std::errc())
			{
				std::cout << "Unable to parse tcp port \"" << argv[i + 1] << "\".\n";
				return {};
			}
			i++;
			continue;
		}

		if (std::strcmp(argv[i], "-UDPPort") == 0)
		{
			if (!hasNext)
			{
				std::cout << "Error: -UDPPort not followed by a value.\n";
				return {};
			}

			auto [p, ec] = std::from_chars(argv[i + 1], argv[i + 1] + std::strlen(argv[i + 1]), ret.UDPPort);
			if (ec != std::errc())
			{
				std::cout << "Unable to parse udp port \"" << argv[i + 1] << "\".\n";
				return {};
			}
			i++;
			continue;
		}

		if (std::strcmp(argv[i], "-m") == 0)
		{
			if (ret.contentType != CMDOptions::contents::none)
			{
				std::cout << "Error: Multiple content specifiers provided.\n";
				return {};
			}

			if (!hasNext)
			{
				std::cout << "Error: -m not followed by a value.\n";
				return {};
			}

			ret.data = argv[i + 1];
			ret.contentType = CMDOptions::contents::message;
			i++;
			continue;
		}

		if (std::strcmp(argv[i], "-f") == 0)
		{
			if (ret.contentType != CMDOptions::contents::none)
			{
				std::cout << "Error: Multiple content specifiers provided.\n";
				return {};
			}

			if (!hasNext)
			{
				std::cout << "Error: -f not followed by a value.\n";
				return {};
			}

			ret.data = argv[i + 1];
			ret.contentType = CMDOptions::contents::file;
			i++;
			continue;
		}

		if (std::strcmp(argv[i], "-freq") == 0)
		{
			if (!hasNext)
			{
				std::cout << "Error: -freq not followed by a value.\n";
				return {};
			}

			auto [p, ec] = std::from_chars(argv[i + 1], argv[i + 1] + std::strlen(argv[i + 1]), ret.frequency);
			if (ec != std::errc())
			{
				std::cout << "Unable to parse frequency value \"" << argv[i + 1] << "\".\n";
				return {};
			}
			i++;
			continue;
		}

		std::cout << "Error: Unknown flag \"" << argv[i] << "\".\n";
		return {};
	}
	return ret;
}

[[nodiscard]]
bool writeToFile(const std::string& path, std::string_view message)
{
	std::ofstream out(path);
	if (!out)
	{
		return false;
	}
	out << message;
	return true;
}

std::optional<std::string> readFromFile(const std::string& path)
{
	std::ifstream in(path, std::ios::binary | std::ios::ate);
	if (!in)
	{
		return {};
	}
	std::streamsize size = in.tellg();
	in.seekg(0, std::ios::beg);

	std::string contents;
	contents.resize(size);
	if (in.read(contents.data(), size))
	{
		return contents;
	}
	return {};
}

void listen(const CMDOptions& opt)
{
	UDPReceiver responder(opt.UDPPort);

	uint16_t port = 0;
	asio::ip::udp::endpoint endpoint;

	std::cout << "Listening on UDP port " << opt.UDPPort << ".\n";

	std::cout << "Found addresses:\n";

	std::set<asio::ip::address_v4> foundAddresses;

	while (true)
	{
		auto maybeRes = responder.awaitRequest(opt.frequency);
		if (!maybeRes)
			continue;
		const auto& res = maybeRes.value();

		if (res.data.getRequest() == UDPRequest::requestAddress)
		{
			if (foundAddresses.insert(res.endpoint.address().to_v4()).second)
			{
				std::cout << "\t" << res.endpoint.address().to_string() << "\n";
			}
			responder.respond(res.endpoint, UDPRequest::respondAddress);
			continue;
		}

		if (res.data.getRequest() == UDPRequest::requestLink)
		{
			std::cout << res.endpoint.address().to_string() << " would like to send data over TCP port " << res.data.getAdditional() << ".\nAccept? [Y/N]\n";
			char v;
			do
			{
				std::cin >> v;
			} while (std::toupper(static_cast<unsigned char>(v)) != 'Y' && std::toupper(static_cast<unsigned char>(v)) != 'N');

			if (v == 'Y')
			{
				port = res.data.getAdditional();
				endpoint = res.endpoint;
				break;
			}
			else
			{
				responder.respond(res.endpoint, UDPRequest::denyLink);
			}
		}
	}

	std::cout << "Awaiting link on TCP port " << port << ".\n";

	TCPReceiver listener(port);

	while (true)
	{
		responder.respond(endpoint, UDPRequest::acceptLink);
		if (listener.awaitAccept(opt.frequency))
		{
			break;
		}
		std::cout << ".";
	}
	std::cout << "\nLink accepted.\n";
	std::cout << "Awaiting data.\n";

	while (true)
	{
		const auto mv = listener.awaitMessage(opt.frequency);
		if (!mv)
		{
			std::cout << ".";
		}
		else
		{
			std::cout << "\nMessage recieved.\n";
			if (opt.contentType == CMDOptions::contents::file)
			{
				if (!writeToFile(opt.data, mv.value().get()))
				{
					std::cout << "Unable to write to file \"" << opt.data << ".\nDumping contents:\n" << mv.value().get() << "\n";
				}
			}
			else
			{
				std::cout << mv.value().get();
			}
			std::cout << "\nDone." << std::endl;
			return;
		}
	}
}

void broadcast(const CMDOptions& opt)
{
	UDPSender broadcast(opt.UDPPort);

	std::cout << "Starting broadcast on UDP port " << opt.UDPPort << ".\n";

	std::cout << "Found addresses:\n";

	std::set<asio::ip::address_v4> foundAddresses;

	broadcast.requestAddress();
	while (true)
	{
		const auto maybeAddress = broadcast.awaitResponse(opt.frequency);
		if (!maybeAddress)
		{
			broadcast.requestAddress();
		}
		else
		{
			if (foundAddresses.insert(maybeAddress.value().endpoint.address().to_v4()).second)
			{
				std::cout << "\t" << maybeAddress.value().endpoint.address().to_string() << "\n";
			}
		}
	}
}

void send(const CMDOptions& opt)
{
	//Fail early if the file is unreadable
	std::string message;
	if (opt.contentType == CMDOptions::contents::file)
	{
		const auto mc = readFromFile(opt.data);
		if (!mc)
		{
			std::cout << "Unable to read file contents from \"" << opt.data << "\".\n";
			return;
		}
		message = mc.value();
	}

	UDPSender linkAsk(opt.UDPPort);

	std::cout << "Requesting link using UDP port " << opt.UDPPort << " to send over TCP port " << opt.TCPPort << ".\n";

	while (true)
	{
		linkAsk.requestLink(opt.address, opt.TCPPort);
		const auto res = linkAsk.awaitResponse(opt.frequency);
		if (res)
		{
			if (res.value().data.getRequest() == UDPRequest::acceptLink)
			{
				std::cout << "\nLink accepted.\n";
				break;
			}
			if (res.value().data.getRequest() == UDPRequest::denyLink)
			{
				std::cout << "\nLink denied.\n";
				return;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(opt.frequency));
		}
		std::cout << ".";
	}

	TCPSender sender;
	std::cout << "Connecting to " << opt.address.to_string() << "...\n";
	while (true)
	{
		if (sender.connect(opt.address.to_string(), std::to_string(opt.TCPPort), opt.frequency))
		{
			break;
		}
		std::cout << ".";
	}

	std::cout << "Connected.\n";
	std::cout << "Sending data...\n";

	if (opt.contentType == CMDOptions::contents::file)
	{
		sender.send(message);
	}
	else
	{
		sender.send(opt.data);
	}

	std::cout << "Done.\n";
}

bool validate(const CMDOptions& opt)
{
	if (opt.progMode == CMDOptions::mode::none)
	{
		std::cout << "Error: No mode set.\n";
		return false;
	}
	if (opt.progMode == CMDOptions::mode::send &&
			(opt.contentType == CMDOptions::contents::none ||
			opt.data.empty()))
	{
		std::cout << "Error: Send mode set but no content provided.\n";
		return false;
	}
	return true;
}

int main(int argc, char** argv)
{
	const auto maybeOptions = parse(argc, argv);
	if (!maybeOptions || !validate(maybeOptions.value()))
	{
		printOptions();
		return 1;
	}

	const auto& options = maybeOptions.value();
	switch (options.progMode)
	{
	case(CMDOptions::mode::listen):
		listen(options);
		break;
	case(CMDOptions::mode::broadcast):
		broadcast(options);
		break;
	case(CMDOptions::mode::send):
		send(options);
		break;
	default:
		std::cout << "Logical error: Invalid case after validate().\n";
		return 2;
	}
	return 0;
}