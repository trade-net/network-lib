#include <tcp_async_server.h>
#include <iostream>

int main()
{
	try
	{
		boost::asio::io_context io_context;
		TcpAsyncServer server(io_context);
		io_context.run();
	}

	catch(std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}
