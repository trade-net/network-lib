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

	// a read can hold part of a frame, one frame, or several, so buffer and extract
	s_inbound.append(data, bytes_transferred);

	if(processFrames())
	{
		start();
	}
}

bool TcpSession::processFrames()
{
	size_t offset = 0;

	while(s_inbound.size() - offset >= kFrameHeaderSize)
	{
		const uint32_t length = readFrameLength(s_inbound.data() + offset);
		if(length == 0 || length > kMaxFrameSize)
		{
			// the stream can't be resynchronised after a bad length
			std::cerr << "error: invalid frame length " << length << ", closing session" << std::endl;
			s_socket.close();
			return false;
		}

		const size_t frameEnd = offset + kFrameHeaderSize + length;
		if(s_inbound.size() < frameEnd)
		{
			break; // wait for the rest of the frame
		}

		const char* body = s_inbound.data() + offset + kFrameHeaderSize;
		const int requestId = static_cast<unsigned char>(body[0]);
		std::string requestData(body + 1, length - 1);
		offset = frameEnd;

		if(requestId == kCloseRequestId)
		{
			// acknowledge, then close once the reply is flushed; ignore anything after it
			s_closeAfterWrite = true;
			write(makeResponse(ResponseStatus::Ok, std::string(1, static_cast<char>(kCloseRequestId))));
			s_inbound.clear();
			return false;
		}

		s_processor.processRequest(
			requestId,
			requestData,
			[self = shared_from_this()](std::string response){
				self->write(std::move(response));
			}
		);
	}

	s_inbound.erase(0, offset);
	return true;
}

void TcpSession::write(std::string response)
{
	boost::asio::post(
		s_socket.get_executor(),
		[self = shared_from_this(), response = std::move(response)]() mutable {
			self->s_writeQueue.push_back(makeFrame(response));
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
