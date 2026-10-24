#ifndef INCLUDED_REQUEST_PROCESSOR_BASE
#define INCLUDED_REQUEST_PROCESSOR_BASE

#include <google/protobuf/message.h>
#include <protocol.h>
#include <vector>
#include <functional>
#include <type_traits>
#include <iostream>

namespace network{

// Handlers run inline on the calling event loop thread, so they must be cheap
// and must not block. Handlers that need to do real work should hand it off
// (e.g. enqueue onto a threadpool) and return.
class RequestProcessor{
public:
	virtual ~RequestProcessor() = default;

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

	// Always calls onComplete exactly once, with an Ok response or an Error
	// response (unknown request id, unparseable request, handler failure).
	// Returns true if the request was handled successfully.
	bool processRequest(int requestId, const std::string& requestData, const std::function<void(std::string)>& onComplete)
	{
		bool ok = false;
		std::string payload;

		if(requestId < 0 || static_cast<size_t>(requestId) >= s_handlers.size())
		{
			payload = "Unknown request id " + std::to_string(requestId);
		}
		else
		{
			try
			{
				payload = s_handlers[requestId](requestData);
				ok = true;
			}
			catch(const std::exception& e)
			{
				payload = e.what();
			}
		}

		if(!ok)
		{
			std::cerr << "Request id " << requestId << " failed: " << payload << std::endl;
		}

		onComplete(makeResponse(ok ? ResponseStatus::Ok : ResponseStatus::Error, payload));
		return ok;
	}

private:
	std::vector<RequestHandler> s_handlers;
};

}
#endif
