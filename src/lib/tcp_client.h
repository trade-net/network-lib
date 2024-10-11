#ifndef INCLUDED_TCP_CLIENT
#define INCLUDED_TCP_CLIENT

#include <boost/asio.hpp>
#include <iostream>
#include <string>

using namespace boost::asio;
using ip::tcp;

namespace network{

class TcpClient {
public:
    TcpClient(const std::string& ip, const std::string& port);

    std::string send(int requestId, const std::string& request);

private:
    void connect(const tcp::resolver::results_type& endpoints);

    boost::asio::io_context s_io_context;
    tcp::socket s_socket;
};

}

#endif
