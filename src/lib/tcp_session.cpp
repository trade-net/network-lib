#include <tcp_session.h>
#include <iostream>

namespace network{

TcpSession::TcpSession(boost::asio::io_context& io_context, RequestProcessor& processor)
: s_socket(io_context)
, s_processor(processor)
{}

TcpSession::sessionPtr TcpSession::create(boost::asio::io_context& io_context, RequestProcessor& processor)
{
	return sessionPtr(new TcpSession(io_context, processor));
}

tcp::socket& TcpSession::socket()
{
	return s_socket;
}


void TcpSession::start()
{
	s_socket.async_read_some(
		boost::asio::buffer(data, MAX_DATA_LENGTH),
		boost::bind(
			&TcpSession::handleRead,
			shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred
		)
	);
}

void TcpSession::handleRead(const boost::system::error_code& ec, size_t bytes_transferred)
{
	if(!ec)
	{

		if(data[0] == 127)
		{
			s_socket.async_write_some(
				boost::asio::buffer(data, sizeof(data)),
				boost::bind(
					&TcpSession::handleWrite,
					shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred
				)
			);
			return;
		}

		int requestId = data[0]; // first byte is the requestId
		std::string requestData(data + 1, bytes_transferred - 1); // the request is in the remaining string

		s_processor.processRequest(
			requestId,
			requestData, 
			[this](const std::string& response){
				s_socket.async_write_some(
					boost::asio::buffer(response, sizeof(response)),
					boost::bind(
						&TcpSession::handleWrite,
						shared_from_this(),
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred
					)
				);
			}
		);
		start();
	}
	else
	{
		std::cerr << "error: " << ec.message() << std::endl;
		s_socket.close();
	}
}

void TcpSession::handleWrite(const boost::system::error_code& ec, size_t bytes_transferred)
{
	if(ec)
	{
		std::cerr << "error: " << ec.message() << std::endl;
		s_socket.close();
	}

}

}
