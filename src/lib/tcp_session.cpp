#include <tcp_session.h>
#include <iostream>

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

void TcpSession::handleRead(const boost::system::error_code& ec, size_t bytes_transformed)
{
	if(!ec)
	{
		std::string requestData(data, bytes_transformed);
		std::string requestType = "getOrder";

		s_processor.processRequest(
			requestType,
			requestData, 
			[this](const std::string& response){
				boost::system::error_code ec;
				boost::asio::write(
					s_socket,
					boost::asio::buffer(response),
					ec
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
