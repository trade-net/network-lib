#ifndef INCLUDED_TCP_ASYNC_SERVER
#define INCLUDED_TCP_ASYNC_SERVER

#include <tcp_session.h>
#include <request_processor.h>

class TcpAsyncServer
{
public:
	TcpAsyncServer(boost::asio::io_context& io_context, RequestProcessor& s_processor);

	void handleAccept(TcpSession::sessionPtr session, const boost::system::error_code& ec);

private:
	void startAccept();

	tcp::acceptor s_acceptor;
	boost::asio::io_context& s_io_context;
	RequestProcessor& s_processor;
};


#endif
