#ifndef INCLUDED_REQUEST_PROCESSOR_BASE
#define INCLUDED_REQUEST_PROCESSOR_BASE

#include <google/protobuf/message.h>
#include <vector>
#include <threadpool.h>
#include <type_traits>
#include <iostream>

namespace network{

class RequestProcessor{
public:
	RequestProcessor(size_t poolSize): threadPool(poolSize){}

	using RequestHandler = std::function<std::string(const std::string&)>;

	template <typename Request, typename Response>
	void registerHandler(std::function<Response(Request)> handler)
	{
		s_handlers.emplace_back([handler](const std::string& serialisedRequest) -> std::string{
			Request request;
			if(!request.ParseFromString(serialisedRequest))
			{
				throw std::runtime_error("Failed to parse Protobuf message");
			}

			Response response = handler(request);

			if constexpr(std::is_same<Response, int>::value)
			{
				return std::to_string(response);
			}
			else
			{
				return response.SerializeAsString();
			}
		});
	}

	void processRequest(int requestId, const std::string& requestData, std::function<void(const std::string&)> onComplete)
	{
		threadPool.enqueue([handler = s_handlers[requestId], requestData, onComplete]{
			std::string response = handler(requestData);
			onComplete(response);
		});
	}

private:
	ThreadPool threadPool;
	std::vector<RequestHandler> s_handlers;
};

}
#endif
