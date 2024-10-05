#include <tcp_async_server.h>
#include <iostream>
#include <my_request_processor.h>

int main()
{
	try
	{
		MyRequestProcessor processor(4);
		TcpAsyncServer server(processor);
		server.run();
	}

	catch(std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}

