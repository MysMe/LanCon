#pragma once
#include "UDPBroadcaster.h"

struct packedBroadcaster
{
	asio::io_service service;
	UDPBroadcaster broadcaster;

	packedBroadcaster(unsigned short port) : broadcaster(service, port) {}
};

struct packedData
{
	asio::ip::address source;
	UDPDataHandler data;
};

packedBroadcaster* UDPB_new(unsigned short port)
{
	return new packedBroadcaster(port);
}

void UDPB_delete(packedBroadcaster* obj)
{
	delete obj;
}

void UDPB_request_addresses(packedBroadcaster* obj)
{
	obj->broadcaster.requestAddress();
}

void UDPB_request_link(packedBroadcaster* obj, asio::ip::address* address, uint16_t linkPort)
{
	obj->broadcaster.requestLink(*address, linkPort);
}

packedData* UDPB_await_response(packedBroadcaster* obj, unsigned int timeoutMS)
{
	if (obj->service.run_one_for(std::chrono::milliseconds(timeoutMS)) != 0)
	{
		packedData* ret = new packedData;
	}
}