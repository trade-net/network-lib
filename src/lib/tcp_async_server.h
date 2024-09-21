#ifndef INCLUDED_TCP_ASYNC_SERVER
#define INCLUDED_TCP_ASYNC_SERVER

#include <tcp_connection_handler.h>

class TcpAsyncServer
{
public:
	using ReadHandler = std::function<std::string(const std::string&)>;

	TcpAsyncServer(boost::asio::io_context& io_context, ReadHandler handler);

	void handleAccept(ConnectionHandler::connPtr connection, const boost::system::error_code& ec);

private:
	void startAccept();

	tcp::acceptor s_acceptor;
	boost::asio::io_context& s_io_context;
	ReadHandler s_readHandler;
};


#endif
