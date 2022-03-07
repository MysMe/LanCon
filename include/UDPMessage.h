#pragma once
#include <asio.hpp>
#include "UDPRequests.h"

//A combination of data and a sender
struct UDPMessage
{
	asio::ip::udp::endpoint endpoint;
	UDPDataHandler data;
};