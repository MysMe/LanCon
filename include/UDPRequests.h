#pragma once
#include <cstdint>
#include <asio.hpp>

enum class UDPRequest : uint8_t
{
	undefined,
	requestAddress,
	respondAddress,
	requestLink,
	acceptLink,
	denyLink
};

//Stores a single UDPRequest and an additional data segment for sending over UDP endpoints
class UDPDataHandler
{
public:
	using additional_t = uint16_t;

private:

	static constexpr std::size_t additionalDataSize = sizeof(additional_t);

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