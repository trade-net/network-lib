#ifndef INCLUDED_REQUEST_PROCESSOR_BASE
#define INCLUDED_REQUEST_PROCESSOR_BASE

//#include <google/protobuf/message.h>
#include <unordered_map>
#include <threadpool.h>

class RequestProcessor{
public:
	RequestProcessor(size_t poolSize): threadPool(poolSize){}

	using RequestHandler = std::function<std::string(const std::string&)>;

	template <typename Request, typename Response>
	void registerHandler(const std::string& requestType, std::function<Response(Request)> handler)
	{
		s_handlers[requestType] = [handler](const std::string& serialisedRequest) -> std::string{
			Request request;
			if(!request.ParseFromString(serialisedRequest))
			{
				throw std::runtime_error("Failed to parse Protobuf message");
			}

			Response response = handler(request);

			return response.SerializeAsString();
		};
	}

	void processRequest(const std::string& requestType, const std::string& requestData, std::function<void(const std::string&)> onComplete)
	{
		auto it = s_handlers.find(requestType);
		if(it != s_handlers.end())
		{
			threadPool.enqueue([handler = it->second, requestData, onComplete]{
				std::string response = handler(requestData);
				onComplete(response);
			});
			return;

		}
		throw std::runtime_error("No handler implemented for request type");
	}

private:
	ThreadPool threadPool;
	std::unordered_map<std::string, RequestHandler> s_handlers;
};

#endif
