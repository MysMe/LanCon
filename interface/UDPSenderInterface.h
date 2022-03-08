#pragma once
#include "Export.h"
#include "UDPSender.h"

LANCONEXPORT UDPSender* UDPSender_new(unsigned short port)
{
	return new UDPSender(port);
}

LANCONEXPORT void UDPSender_delete(UDPSender* obj)
{
	delete obj;
}

LANCONEXPORT void UDPSender_request_addresses(UDPSender* obj)
{
	obj->requestAddress();
}

LANCONEXPORT void UDPSender_request_link(UDPSender* obj, const char* const target, unsigned short linkPort)
{
	obj->requestLink(asio::ip::address::from_string(target), linkPort);
}

//May return nullptr if no response recieved
LANCONEXPORT UDPMessage* UDPSender_await_response_new(UDPSender* obj, uint16_t timeoutMS)
{
	auto v = obj->awaitResponse(timeoutMS);
	if (!v)
		return nullptr;
	return new UDPMessage(std::move(v.value()));
}