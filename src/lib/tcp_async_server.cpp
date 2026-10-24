#include <tcp_async_server.h>

namespace network{

TcpAsyncServer::TcpAsyncServer(RequestProcessor& processor, size_t numLoops, unsigned short port)
: s_acceptor(s_acceptContext, tcp::endpoint(tcp::v4(), port))
, s_processor(processor)
{
	if(numLoops == 0)
	{
		numLoops = 1;
	}

	for(size_t i = 0; i < numLoops; ++i)
	{
		s_loops.push_back(std::make_unique<boost::asio::io_context>(1));
		s_guards.emplace_back(s_loops.back()->get_executor());
	}

	startAccept();
}

TcpAsyncServer::~TcpAsyncServer()
{
	stop();
}

void TcpAsyncServer::run()
{
	for(auto& loop : s_loops)
	{
		s_threads.emplace_back([&loop]{ loop->run(); });
	}

	s_acceptContext.run();
}

void TcpAsyncServer::stop()
{
	s_acceptContext.stop();
	s_guards.clear();
	for(auto& loop : s_loops)
	{
		loop->stop();
	}
	for(auto& thread : s_threads)
	{
		if(thread.joinable())
		{
			thread.join();
		}
	}
	s_threads.clear();
}

void TcpAsyncServer::startAccept()
{
	auto& loop = *s_loops[s_nextLoop++ % s_loops.size()];
	TcpSession::sessionPtr session = TcpSession::create(loop, s_processor);
	s_acceptor.async_accept(
		session->socket(),
		[this, session, &loop](const boost::system::error_code& ec){
			if(!ec)
			{
				// start the session on its own loop so all its handlers run there
				boost::asio::post(loop, [session]{ session->start(); });
			}
			startAccept();
		}
	);
}

}
