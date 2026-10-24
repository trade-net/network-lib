#ifndef INCLUDED_TCP_ASYNC_SERVER
#define INCLUDED_TCP_ASYNC_SERVER

#include <tcp_session.h>
#include <request_processor.h>

#include <memory>
#include <thread>
#include <vector>

namespace network{

// Accepts connections on the thread that calls run(), and assigns each new
// connection round-robin to one of numLoops event loops. Each loop is driven
// by its own thread and a connection stays on its loop for its whole life.
class TcpAsyncServer
{
public:
	TcpAsyncServer(RequestProcessor& processor, size_t numLoops = 1, unsigned short port = 1234);
	~TcpAsyncServer();

	// Blocks until stop() is called.
	void run();
	void stop();

private:
	using WorkGuard = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;

	void startAccept();

	boost::asio::io_context s_acceptContext;
	tcp::acceptor s_acceptor;
	RequestProcessor& s_processor;

	std::vector<std::unique_ptr<boost::asio::io_context>> s_loops;
	std::vector<WorkGuard> s_guards;
	std::vector<std::thread> s_threads;
	size_t s_nextLoop = 0;
};

}

#endif
