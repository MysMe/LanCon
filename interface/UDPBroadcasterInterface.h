#pragma once
#include "UDPBroadcaster.h"
#include "Export.h"

LANCONEXPORT UDPBroadcaster* broadcaster_new(unsigned short port)
{
	return new UDPBroadcaster(port);
}

LANCONEXPORT void broadcaster_delete(UDPBroadcaster* obj)
{
	delete obj;
}

LANCONEXPORT void broadcaster_request_addresses(UDPBroadcaster* obj)
{
	obj->requestAddress();
}

LANCONEXPORT void broadcaster_request_link(UDPBroadcaster* obj, const char* const target, unsigned short linkPort)
{
	obj->requestLink(asio::ip::address::from_string(target), linkPort);
}

//May return nullptr if no response recieved
LANCONEXPORT UDPMessage* broadcaster_await_response_new(UDPBroadcaster* obj, uint16_t timeoutMS)
{
	const auto v = obj->awaitResponse(timeoutMS);
	if (!v)
		return nullptr;
	return new UDPMessage(std::move(v.value()));
}