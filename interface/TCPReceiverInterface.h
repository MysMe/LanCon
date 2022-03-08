#pragma once
#include "Export.h"
#include "TCPReceiver.h"

LANCONEXPORT TCPListener* TCPListener_new(unsigned short port)
{
	return new TCPListener(port);
}

LANCONEXPORT void TCPListener_delete(TCPListener* obj)
{
	delete obj;
}

//May return nullptr
LANCONEXPORT char* TCPListener_await_accept_new(TCPListener* obj, uint16_t timeoutMS)
{
	const std::optional<asio::ip::address> v = obj->awaitAccept(timeoutMS);
	if (!v)
		return nullptr;

	const auto& str = v.value().to_string();

	char* ret = new char[str.size() + 1];
	std::memcpy(ret, str.data(), str.size());
	ret[str.size()] = '\0';
	return ret;
}

LANCONEXPORT void TCPListener_string_delete(char* str)
{
	delete[] str;
}

LANCONEXPORT char* TCPListener_await_message(TCPListener* obj, uint16_t timeoutMS)
{
	const auto v = obj->awaitMessage(timeoutMS);
	if (!v)
		return nullptr;

	const auto& str = v.value().get();
	char* ret = new char[str.size() + 1];
	std::memcpy(ret, str.data(), str.size());
	ret[str.size()] = '\0';
	return ret;
}

LANCONEXPORT void TCPListener_clear_message(TCPListener* obj)
{
	obj->clearBuffer();
}
