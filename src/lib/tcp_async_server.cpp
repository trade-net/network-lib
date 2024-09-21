#include <tcp_async_server.h>


TcpAsyncServer::TcpAsyncServer(boost::asio::io_context& io_context)
: s_acceptor(io_context, tcp::endpoint(tcp::v4(), 1234))
, s_io_context(io_context)
{
	startAccept();
}

void TcpAsyncServer::handleAccept(
	ConnectionHandler::connPtr connection, const boost::system::error_code& err
)
{
	if(!err)
	{
		connection->start();
	}
	startAccept();
}

void TcpAsyncServer::startAccept()
{
	ConnectionHandler::connPtr connection = ConnectionHandler::create(s_io_context);
	s_acceptor.async_accept(
		connection->socket(),
		boost::bind(
			&TcpAsyncServer::handleAccept,
			this,
			connection,
			boost::asio::placeholders::error
		)
	);
}

