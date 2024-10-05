#ifndef INCLUDED_TCP_ASYNC_SERVER
#define INCLUDED_TCP_ASYNC_SERVER

#include <tcp_session.h>
#include <request_processor.h>

class TcpAsyncServer
{
public:
	TcpAsyncServer(RequestProcessor& s_processor);

	void handleAccept(TcpSession::sessionPtr session, const boost::system::error_code& ec);

	void run()
	{
		s_io_context.run();
	}

private:
	void startAccept();
	boost::asio::io_context s_io_context;
	tcp::acceptor s_acceptor;
	RequestProcessor& s_processor;
};


#endif
