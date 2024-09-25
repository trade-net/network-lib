#include <tcp_connection_handler.h>
#include <iostream>

ConnectionHandler::ConnectionHandler(boost::asio::io_context& io_context, RequestProcessor& processor)
: s_socket(io_context)
, s_processor(processor)
{}

ConnectionHandler::connPtr ConnectionHandler::create(boost::asio::io_context& io_context, RequestProcessor& processor)
{
	return connPtr(new ConnectionHandler(io_context, processor));
}

tcp::socket& ConnectionHandler::socket()
{
	return s_socket;
}


void ConnectionHandler::start()
{
	s_socket.async_read_some(
		boost::asio::buffer(data, MAX_DATA_LENGTH),
		boost::bind(
			&ConnectionHandler::handleRead,
			shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred
		)
	);
}

void ConnectionHandler::handleRead(const boost::system::error_code& ec, size_t bytes_transformed)
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
