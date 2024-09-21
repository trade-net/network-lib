#include <tcp_async_server.h>
#include <iostream>

std::string handler(const std::string& inp)
{
	std::cout << "Printing data from handler: " << inp << std::endl;
	return "1";
}

int main()
{
	try
	{
		boost::asio::io_context io_context;
		TcpAsyncServer server(io_context, handler);
		io_context.run();
	}

	catch(std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}

