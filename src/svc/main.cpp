#include <tcp_async_server.h>
#include <iostream>
#include <my_request_processor.h>

int main()
{
	try
	{
		boost::asio::io_context io_context;
		MyRequestProcessor processor(4);
		TcpAsyncServer server(io_context, processor);
		io_context.run();
	}

	catch(std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}

