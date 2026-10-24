#ifndef INCLUDED_TCP_CONNECTION_HANDLER
#define INCLUDED_TCP_CONNECTION_HANDLER

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <request_processor.h>

#include <deque>
#include <string>

#define MAX_DATA_LENGTH 1024

namespace network{

using namespace boost::asio;
using ip::tcp;

// All of a session's handlers run on the io_context it was created with, which
// is driven by a single thread, so session state needs no locking.
class TcpSession : public boost::enable_shared_from_this<TcpSession>
{
public:
	using sessionPtr = boost::shared_ptr<TcpSession>;

	TcpSession(boost::asio::io_context& io_context, RequestProcessor& processor);

	static sessionPtr create(boost::asio::io_context& io_context, RequestProcessor& processor);

	tcp::socket& socket();

	void start();
	void handleRead(const boost::system::error_code& ec, size_t bytes_transferred);

	// Safe to call from any thread: the write is posted onto the session's loop.
	void write(std::string response);

private:
	void doWrite();
	void handleWrite(const boost::system::error_code& ec, size_t bytes_transferred);

	tcp::socket s_socket;
	char data[MAX_DATA_LENGTH];

	RequestProcessor& s_processor;

	std::deque<std::string> s_writeQueue;
	bool s_writing = false;
	bool s_closeAfterWrite = false;
};

}

#endif
