#ifndef INCLUDED_TCP_CONNECTION_HANDLER
#define INCLUDED_TCP_CONNECTION_HANDLER

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>

#define MAX_DATA_LENGTH 1024

using namespace boost::asio;
using ip::tcp;

class ConnectionHandler : public boost::enable_shared_from_this<ConnectionHandler>
{
public:
	using connPtr = boost::shared_ptr<ConnectionHandler>;
	using ReadHandler = std::function<std::string(const std::string&)>;

	ConnectionHandler(boost::asio::io_context& io_context, ReadHandler handler);

	static connPtr create(boost::asio::io_context& io_context, ReadHandler handler);

	tcp::socket& socket();

	void start();
	void handleRead(const boost::system::error_code& ec, size_t bytes_transferred);

private:
	tcp::socket s_socket;
	char data[MAX_DATA_LENGTH];

	ReadHandler s_readHandler;

};


#endif
