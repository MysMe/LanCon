#pragma once
#include "Export.h"
#include "UDPMessage.h"

LANCONEXPORT void UDPMessage_delete(UDPMessage* obj)
{
	delete obj;
}

LANCONEXPORT char* UDPMessage_address_new(UDPMessage* obj)
{
	const auto& str = obj->endpoint.address().to_string();
	char* ret = new char[str.size() + 1];
	std::memcpy(ret, str.data(), str.size());
	ret[str.size()] = '\0';
	return ret;
}

LANCONEXPORT void UDPMessage_address_delete(const char* str)
{
	delete[] str;
}

LANCONEXPORT uint8_t UDPMessage_request(UDPMessage* obj)
{
	return static_cast<uint8_t>(obj->data.getRequest());
}

LANCONEXPORT uint16_t UDPMessage_additional(UDPMessage* obj)
{
	return obj->data.getAdditional();
}