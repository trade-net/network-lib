#include <tcp_async_server.h>


TcpAsyncServer::TcpAsyncServer(RequestProcessor& processor)
: s_acceptor(s_io_context, tcp::endpoint(tcp::v4(), 1234))
, s_processor(processor)
{
	startAccept();
}

void TcpAsyncServer::handleAccept(
	TcpSession::sessionPtr session, const boost::system::error_code& ec
)
{
	if(!ec)
	{
		session->start();
	}
	startAccept();
}

void TcpAsyncServer::startAccept()
{
	TcpSession::sessionPtr session = TcpSession::create(s_io_context, s_processor);
	s_acceptor.async_accept(
		session->socket(),
		boost::bind(
			&TcpAsyncServer::handleAccept,
			this,
			session,
			boost::asio::placeholders::error
		)
	);
}

