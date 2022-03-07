#pragma once
#include "UDPListener.h"
#include "Export.h"

LANCONEXPORT UDPResponder* UDPResponder_new(unsigned short port)
{
	return new UDPResponder(port);
}

LANCONEXPORT void UDPResponder_delete(UDPResponder* obj)
{
	delete obj;
}

//May return nullptr if no request recieved
LANCONEXPORT UDPMessage* UDPResponder_await_message_new(UDPResponder* obj, uint16_t timeoutMS)
{
	auto v = obj->awaitRequest(timeoutMS);
	if (!v)
		return nullptr;
	
	return new UDPMessage(std::move(v.value()));
}

LANCONEXPORT void UDPResponder_respond(UDPResponder* obj, UDPMessage* respondTo, uint8_t request)
{
	obj->respond(respondTo->endpoint, static_cast<UDPRequest>(request));
}