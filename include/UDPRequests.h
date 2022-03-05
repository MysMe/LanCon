#pragma once
#include <cstdint>
#include <asio.hpp>

enum class UDPRequest : uint8_t
{
	undefined, //No use, any time this is seen is an error
	requestAddress, //Broadcast from a device searching for other devices
	respondAddress, //Response given when a device has been asked for its adress
	requestLink, //Sent from a device looking to open a TCP connection
	acceptLink, //Positive response to open a TCP connection
	denyLink //Negative response to open a TCP connection
};

//Stores a single UDPRequest and an additional data segment for sending over UDP endpoints
class UDPDataHandler
{
public:
	//Additional data sent with a request
	using additional_t = uint16_t;

private:

	static constexpr std::size_t additionalDataSize = sizeof(additional_t);

	//Both request and data in a single array
	std::array<std::byte, sizeof(UDPRequest) + additionalDataSize> buffer{};
public:

	UDPDataHandler() = default;
	UDPDataHandler(UDPRequest req)
	{
		setRequest(req);
	}
	UDPDataHandler(UDPRequest req, additional_t data)
	{
		setRequest(req);
		setAdditional(data);
	}

	void setRequest(UDPRequest request)
	{
		std::memcpy(buffer.data(), &request, sizeof(UDPRequest));
	}

	void setAdditional(additional_t data)
	{
		std::memcpy(buffer.data() + sizeof(UDPRequest), &data, additionalDataSize);
	}

	UDPRequest getRequest() const
	{
		UDPRequest ret;
		std::memcpy(&ret, buffer.data(), sizeof(UDPRequest));
		return ret;
	}

	additional_t getAdditional() const
	{
		additional_t ret;
		std::memcpy(&ret, buffer.data() + sizeof(UDPRequest), additionalDataSize);
		return ret;
	}

	auto getBuffer()
	{
		return asio::buffer(buffer);
	}

};