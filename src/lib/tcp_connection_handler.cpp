#include <tcp_connection_handler.h>
#include <iostream>

ConnectionHandler::ConnectionHandler(boost::asio::io_context& io_context, ReadHandler handler)
: s_socket(io_context)
, s_readHandler(handler)
{}

ConnectionHandler::connPtr ConnectionHandler::create(boost::asio::io_context& io_context, ReadHandler handler)
{
	return connPtr(new ConnectionHandler(io_context, handler));
}

tcp::socket& ConnectionHandler::socket()
{
	return s_socket;
}


void ConnectionHandler::start()
{
	s_socket.async_read_some(
		boost::asio::buffer(data, MAX_DATA_LENGTH),
		/*
		[this](boost::system::error_code ec, std::size_t length)
		{
			if(!ec)
			{
				std::string request(data, length);
				if(request != "0")
				{
					std::cout << request << std::endl;
					std::string response = "1";
					std::cout << response << std::endl;
					boost::system::error_code write_ec;
					boost::asio::write(
						s_socket,
						boost::asio::buffer(response),
						write_ec
					);
					start();
				}
				else
				{
					boost::system::error_code write_ec;
					boost::asio::write(
						s_socket,
						boost::asio::buffer("0"),
						write_ec
					);

				}

			}
			else
			{
				std::cerr << "error: " << ec.message() << std::endl;
				s_socket.close();
			}

		}
		*/
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
		std::string request(data, bytes_transformed);
		boost::system::error_code response_ec;


		if(request != "0")
		{
			std::string response = s_readHandler(request);
			boost::asio::write(
				s_socket,
				boost::asio::buffer(response),
				response_ec
			);
			start();
		}
		else
		{
			std::cout << "Connection ending" << std::endl;
			boost::asio::write(
				s_socket,
				boost::asio::buffer("0"),
				response_ec
			);
		}
	}
	else
	{
		std::cerr << "error: " << ec.message() << std::endl;
		s_socket.close();
	}
}
