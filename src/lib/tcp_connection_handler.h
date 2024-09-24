#ifndef INCLUDED_TCP_CONNECTION_HANDLER
#define INCLUDED_TCP_CONNECTION_HANDLER

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <request_processor.h>

#define MAX_DATA_LENGTH 1024

using namespace boost::asio;
using ip::tcp;

class ConnectionHandler : public boost::enable_shared_from_this<ConnectionHandler>
{
public:
	using connPtr = boost::shared_ptr<ConnectionHandler>;

	ConnectionHandler(boost::asio::io_context& io_context, RequestProcessor& processor);

	static connPtr create(boost::asio::io_context& io_context, RequestProcessor& processor);

	tcp::socket& socket();

	void start();
	void handleRead(const boost::system::error_code& ec, size_t bytes_transferred);

private:
	tcp::socket s_socket;
	char data[MAX_DATA_LENGTH];

	RequestProcessor& s_processor;

};


#endif
