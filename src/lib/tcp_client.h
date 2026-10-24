#ifndef INCLUDED_TCP_CLIENT
#define INCLUDED_TCP_CLIENT

#include <boost/asio.hpp>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace boost::asio;
using ip::tcp;

namespace network{

// Thrown by TcpClient::send when the server replies with an Error status.
class RequestError : public std::runtime_error{
public:
    using std::runtime_error::runtime_error;
};

class TcpClient {
public:
    TcpClient(const std::string& ip, const std::string& port);

    // Returns the response payload. Throws RequestError if the server rejected the request.
    std::string send(const std::string& request);
    std::string send(int requestId, const std::string& request);

private:
    void connect(const tcp::resolver::results_type& endpoints);

    boost::asio::io_context s_io_context;
    tcp::socket s_socket;
};

}

#endif
