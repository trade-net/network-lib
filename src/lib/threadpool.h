#ifndef INCLUDED_THREADPOOL
#define INCLUDED_THREADPOOL

#include <vector>
#include <queue>
#include <mutex>
#include <thread>
#include <functional>
#include <condition_variable>
#include <memory>

class ThreadPool{
public:
	ThreadPool(size_t numThreads);

	void enqueue(std::function<void()> task);
	
	~ThreadPool();

private:
	std::vector<std::thread> workers;
	std::queue<std::function<void()>> taskQueue;
	std::mutex queueMutex;
	std::condition_variable condition;
	bool stop = false;
};

#endif
