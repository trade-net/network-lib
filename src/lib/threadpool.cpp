#include <threadpool.h>
#include <iostream>

namespace network{

ThreadPool::ThreadPool(size_t numThreads)
{
	for(size_t i = 0; i < numThreads; ++i)
	{
		workers.emplace_back([this, i] {
			try{
				while(true)
				{
					std::function<void()> task;
					{
						std::unique_lock<std::mutex> lock(queueMutex);

						condition.wait(lock, [this] {
							return !taskQueue.empty() || stop;
						});

						if(stop && taskQueue.empty())
							return;

						task = std::move(taskQueue.front());
						taskQueue.pop();
					}

					task();
				}
			} catch (const std::exception &e) {
				std::cerr << "Exception in thread " << i << ": " << e.what() << std::endl;
				throw;
			} catch (...) {
				std::cerr << "Unknown exception in thread " << i << std::endl;
			}
		});


	}
}

void ThreadPool::enqueue(std::function<void()> task)
{
	{
		std::unique_lock<std::mutex> lock(queueMutex);
		taskQueue.emplace(std::move(task));
	}
	condition.notify_all();
}

ThreadPool::~ThreadPool()
{
	{
		std::unique_lock<std::mutex> lock(queueMutex);
		stop = true;
	}

	condition.notify_all();

	for(std::thread& worker : workers)
	{
		worker.join();
	}
}

}
