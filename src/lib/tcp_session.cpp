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
	if(ec || bytes_transferred == 0)
	{
		if(ec != boost::asio::error::eof)
		{
			std::cerr << "error: " << ec.message() << std::endl;
		}
		s_socket.close();
		return;
	}

	if(data[0] == 127)
	{
		// acknowledge with the close opcode, then close once the reply is flushed
		s_closeAfterWrite = true;
		write(makeResponse(ResponseStatus::Ok, std::string(1, static_cast<char>(127))));
		return;
	}

	// copy out of the read buffer before re-arming the read, which reuses it
	int requestId = static_cast<unsigned char>(data[0]); // first byte is the requestId
	std::string requestData(data + 1, bytes_transferred - 1); // the request is in the remaining string

	start();

	s_processor.processRequest(
		requestId,
		requestData,
		[self = shared_from_this()](std::string response){
			self->write(std::move(response));
		}
	);
}

void TcpSession::write(std::string response)
{
	boost::asio::post(
		s_socket.get_executor(),
		[self = shared_from_this(), response = std::move(response)]() mutable {
			self->s_writeQueue.push_back(std::move(response));
			if(!self->s_writing)
			{
				self->doWrite();
			}
		}
	);
}

void TcpSession::doWrite()
{
	s_writing = true;
	boost::asio::async_write(
		s_socket,
		boost::asio::buffer(s_writeQueue.front()),
		boost::bind(
			&TcpSession::handleWrite,
			shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred
		)
	);
}

void TcpSession::handleWrite(const boost::system::error_code& ec, size_t bytes_transferred)
{
	s_writeQueue.pop_front();

	if(ec)
	{
		std::cerr << "error: " << ec.message() << std::endl;
		s_socket.close();
		s_writeQueue.clear();
		s_writing = false;
		return;
	}

	if(s_writeQueue.empty())
	{
		s_writing = false;
		if(s_closeAfterWrite)
		{
			s_socket.close();
		}
	}
	else
	{
		doWrite();
	}
}

}
