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

	std::string body(1, static_cast<char>(requestId)); // first byte is requestId
	body += request;
	if(body.size() > kMaxFrameSize)
	{
		throw std::runtime_error("Request too large");
	}

	boost::asio::write(s_socket, boost::asio::buffer(makeFrame(body)), ec);
	if(ec)
	{
		throw std::runtime_error("Error sending message: " + ec.message());
	}

	// the response is a frame too: read the header, then exactly the body
	char header[kFrameHeaderSize];
	boost::asio::read(s_socket, boost::asio::buffer(header), ec);
	if(ec)
	{
		throw std::runtime_error("Error receiving response: " + ec.message());
	}

	const uint32_t length = readFrameLength(header);
	if(length == 0 || length > kMaxFrameSize)
	{
		throw std::runtime_error("Invalid response frame length " + std::to_string(length));
	}

	std::string response(length, '\0');
	boost::asio::read(s_socket, boost::asio::buffer(response), ec);
	if(ec)
	{
		throw std::runtime_error("Error receiving response: " + ec.message());
	}

	std::string payload = response.substr(1);
	if(static_cast<ResponseStatus>(response[0]) != ResponseStatus::Ok)
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

