#pragma once
#include "Export.h"
#include "TCPReceiver.h"

LANCONEXPORT TCPReceiver* TCPReceiver_new(unsigned short port)
{
	return new TCPReceiver(port);
}

LANCONEXPORT void TCPReceiver_delete(TCPReceiver* obj)
{
	delete obj;
}

//May return nullptr
LANCONEXPORT char* TCPReceiver_await_accept_new(TCPReceiver* obj, uint16_t timeoutMS)
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

LANCONEXPORT void TCPReceiver_string_delete(char* str)
{
	delete[] str;
}

//May return nullptr
LANCONEXPORT char* TCPReceiver_await_message_new(TCPReceiver* obj, uint16_t timeoutMS)
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

LANCONEXPORT void TCPReceiver_clear_message(TCPReceiver* obj)
{
	obj->clearBuffer();
}

LANCONEXPORT std::size_t TCPReceiver_get_message_size(TCPReceiver* obj)
{
	return obj->getMessageSize();
}

LANCONEXPORT std::size_t TCPReceiver_get_message_received(TCPReceiver* obj)
{
	return obj->getMessageReceived();
}

LANCONEXPORT bool TCPReceiver_failed(TCPReceiver* obj)
{
	return obj->failed();
}