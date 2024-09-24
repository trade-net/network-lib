#include <tcp_async_server.h>


TcpAsyncServer::TcpAsyncServer(boost::asio::io_context& io_context, RequestProcessor& processor)
: s_acceptor(io_context, tcp::endpoint(tcp::v4(), 1234))
, s_io_context(io_context)
, s_processor(processor)
{
	startAccept();
}

void TcpAsyncServer::handleAccept(
	ConnectionHandler::connPtr connection, const boost::system::error_code& ec
)
{
	if(!ec)
	{
		connection->start();
	}
	startAccept();
}

void TcpAsyncServer::startAccept()
{
	ConnectionHandler::connPtr connection = ConnectionHandler::create(s_io_context, s_processor);
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

