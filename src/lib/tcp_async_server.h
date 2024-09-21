#ifndef INCLUDED_TCP_ASYNC_SERVER
#define INCLUDED_TCP_ASYNC_SERVER

#include <tcp_connection_handler.h>

class TcpAsyncServer
{
public:
	TcpAsyncServer(boost::asio::io_context& io_context);

	void handleAccept(ConnectionHandler::connPtr connection, const boost::system::error_code& err);

private:
	tcp::acceptor s_acceptor;
	boost::asio::io_context& s_io_context;
	void startAccept();
};


#endif
