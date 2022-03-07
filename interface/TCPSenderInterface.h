#pragma once
#include "Export.h"
#include "TCPSender.h"

LANCONEXPORT TCPSender* TCPSender_new()
{
	return new TCPSender();
}

LANCONEXPORT void TCPSender_delete(TCPSender* obj)
{
	delete obj;
}

LANCONEXPORT bool TCPSender_connect(TCPSender* obj, const char* const IP, unsigned short port, uint16_t timeoutMS)
{
	return obj->connect(IP, std::to_string(port), timeoutMS);
}

LANCONEXPORT bool TCPSender_connected(TCPSender* obj)
{
	return obj->connected();
}

LANCONEXPORT void TCPSender_disconnect(TCPSender* obj)
{
	obj->disconnect();
}

LANCONEXPORT void TCPSender_send(TCPSender* obj, const char* const message)
{
	obj->send(message);
}