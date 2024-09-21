#include <tcp_connection_handler.h>
#include <iostream>

ConnectionHandler::ConnectionHandler(boost::asio::io_context& io_context)
: s_socket(io_context)
{}

ConnectionHandler::connPtr ConnectionHandler::create(boost::asio::io_context& io_context)
{
	return connPtr(new ConnectionHandler(io_context));
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

void ConnectionHandler::handleRead(const boost::system::error_code& err, size_t bytes_transformed)
{
	if(!err)
	{
		std::string dataStr(data, bytes_transformed);
		std::cout << dataStr << std::endl;

		boost::system::error_code ec;
		boost::asio::write(
			s_socket,
			boost::asio::buffer("1"),
			ec
		);

		if(dataStr != "0")
		{
			start();
		}
		else
		{
			std::cout << "Connection ending" << std::endl;
		}
	}
	else
	{
		std::cerr << "error: " << err.message() << std::endl;
		s_socket.close();
	}
}
