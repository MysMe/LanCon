#pragma once
#include <asio.hpp>

class serviceBase
{
protected:
	asio::io_service service;

public:
	virtual ~serviceBase() = default;
};
