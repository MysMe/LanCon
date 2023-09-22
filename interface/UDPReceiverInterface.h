#pragma once
#include "Export.h"
#include "UDPReceiver.h"

LANCONEXPORT UDPReceiver* UDPReceiver_new(unsigned short port, const char* address)
{
	return new UDPReceiver(port, address);
}

LANCONEXPORT void UDPReceiver_delete(UDPReceiver* obj)
{
	delete obj;
}

//May return nullptr if no request received
LANCONEXPORT UDPMessage* UDPReceiver_await_message_new(UDPReceiver* obj, uint16_t timeoutMS)
{
	auto v = obj->awaitRequest(timeoutMS);
	if (!v)
		return nullptr;
	
	return new UDPMessage(std::move(v.value()));
}

LANCONEXPORT void UDPReceiver_respond(UDPReceiver* obj, UDPMessage* respondTo, uint8_t request)
{
	obj->respond(respondTo->endpoint, static_cast<UDPRequest>(request));
}