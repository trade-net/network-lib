#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <tcp_client.h>
#include <protocol.h>

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
	if (ec)
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

	if(length == 0)
	{
		return "";
	}

	std::string payload(responseArr.data() + 1, length - 1);
	if(static_cast<ResponseStatus>(responseArr[0]) != ResponseStatus::Ok)
	{
		throw RequestError(payload);
	}

	return payload;
}

void TcpClient::connect(const tcp::resolver::results_type& endpoints)
{
	boost::system::error_code ec;
	boost::asio::connect(s_socket, endpoints, ec);
	if (ec) {
		throw std::runtime_error("Connection failed: " + ec.message());
	}
}

}

