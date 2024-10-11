#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <tcp_client.h>

namespace network{

TcpClient::TcpClient(const std::string& ip, const std::string& port)
: s_socket(s_io_context)
{
	tcp::resolver resolver(s_io_context);
	tcp::resolver::results_type endpoints = resolver.resolve(ip, port);
	connect(endpoints);
}

std::string TcpClient::send(const std::string& request)
{
	return send(0, request);
}

std::string TcpClient::send(int requestId, const std::string& request)
{
	boost::system::error_code ec;

	// Send the message to the server
	boost::asio::write(s_socket, boost::asio::buffer(static_cast<char>(requestId) + request), ec); // first byte is requestId
	if (!ec) 
	{
		std::cout << "Message sent: " << request << std::endl;
	}
	else
	{
		std::cerr << "Error sending message: " << ec.message() << std::endl;
	}

	// Wait for the server's response
	std::array<char, 1024> responseArr;
	size_t length = s_socket.read_some(boost::asio::buffer(responseArr), ec);

	if(ec)
	{
		std::cerr << "Error receiving response: " << ec.message() << std::endl;
		return "";
	}

	return std::string(responseArr.data(), length);
}

void TcpClient::connect(const tcp::resolver::results_type& endpoints)
{
	boost::system::error_code ec;
	boost::asio::connect(s_socket, endpoints, ec);
	if (!ec) {
		std::cout << "Connected to server!" << std::endl;
	} else {
		std::cerr << "Connection failed: " << ec.message() << std::endl;
	}
}

}

