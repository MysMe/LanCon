#pragma once
#include <asio.hpp>

class serviceBase
{
	/*
	The service object must be constructed before objects that rely on it.
	Objects are constructed in inheritance order so by inheriting from this class it guarantees the service is constructed first.
	*/
protected:
	asio::io_service service;

public:
	virtual ~serviceBase() = default;
};
