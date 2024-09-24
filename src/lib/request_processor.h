#ifndef INCLUDED_REQUEST_PROCESSOR_BASE
#define INCLUDED_REQUEST_PROCESSOR_BASE

//#include <google/protobuf/message.h>
#include <map>

class RequestProcessor{
public:
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

	std::string processRequest(const std::string& requestType, const std::string& requestData)
	{
		auto it = s_handlers.find(requestType);
		if(it != s_handlers.end())
		{
			return it->second(requestData);
		}
		throw std::runtime_error("No handler implemented for request type");
	}

private:
	std::map<std::string, RequestHandler> s_handlers;

};

#endif
