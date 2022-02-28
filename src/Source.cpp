#include <asio/io_service.hpp>
#include <asio/write.hpp>
#include <asio/buffer.hpp>
#include <asio/ip/tcp.hpp>
#include <array>
#include <string>
#include <iostream>
#include <functional>

using namespace asio;
using namespace asio::ip;

io_service ioservice;
tcp::resolver resolv{ ioservice };
tcp::socket tcp_socket{ ioservice };
std::array<char, 4096> bytes;

class serviceBase
{
protected:
    asio::io_service service;

public:
    virtual ~serviceBase() = default;
};

class TCPRequest : private serviceBase
{
    std::vector<std::byte> response;
    tcp::resolver resolver;
    tcp::socket socket;

    static constexpr std::size_t bufferSize = 1024;

public:

    TCPRequest(const std::string& destination, const std::string& port) : resolver(service), socket(service)
    {
        resolver.async_resolve(tcp::resolver::query(destination, port),
            std::bind(&TCPRequest::resolve, this,
                std::placeholders::_1, std::placeholders::_2));
    }

    const std::vector<std::byte>& get() const
    {
        return response;
    }

    void read(const asio::error_code& ec, std::size_t bytes)
    {
        if (!ec)
        {
            const std::size_t start = response.size();
            response.resize(response.size() + bytes);
            socket.async_read_some(buffer(response.data() + start, bytes),
                std::bind(&TCPRequest::read, this,
                    std::placeholders::_1, std::placeholders::_2));
            service.stop();
        }
    }

    void connect(const asio::error_code& ec)
    {
        if (!ec)
        {
            const std::size_t start = response.size();
            response.resize(response.size() + bufferSize);

            std::string r =
                "GET / HTTP/1.1\r\nHost: theboostcpplibraries.com\r\n\r\n";
            write(socket, buffer(r));
            socket.async_read_some(buffer(response.data() + start, bufferSize), 
                std::bind(&TCPRequest::read, this,
                    std::placeholders::_1, std::placeholders::_2));
        }
    }

    void resolve(const asio::error_code& ec, tcp::resolver::iterator it)
    {
        if (!ec)
            socket.async_connect(*it, 
                std::bind(&TCPRequest::connect, this,
                    std::placeholders::_1));
    }

    void execute()
    {
        service.run();
    }
};

class TCPConnection
{
    tcp::socket socket;
    std::string message;

    void handle_write(asio::error_code&, size_t) {}

public:
    TCPConnection(asio::io_service& service) : socket(service) {}

    void send(const std::string& msg)
    {
        asio::write(socket, buffer(msg),
            std::bind(&TCPConnection::handle_write, this,
                std::placeholders::_1, std::placeholders::_2));
    }

    tcp::socket& getSocket()
    {
        return socket;
    }
};

class TCPListener : private serviceBase
{
    tcp::acceptor acceptor;

    void start_accept()
    {
        TCPConnection con(service);
        std::cout << "Waiting.";
        acceptor.async_accept(con.getSocket(),
            std::bind(&TCPListener::handle_accept, this, con,
                std::placeholders::_1));
    }

    void handle_accept(TCPConnection& con, const asio::error_code& ec)
    {
        if (!ec)
        {
            con.send("Hello.");
        }
        start_accept();
    }

public:
    TCPListener() : acceptor(service) {}

};

int main()
{
    TCPRequest request("theboostcpplibraries.com", "80");
    request.execute();
    for (const auto i : request.get())
        std::cout << static_cast<char>(i);
    std::cin.ignore();
}