#ifndef INCLUDED_PROTOCOL
#define INCLUDED_PROTOCOL

#include <string>

namespace network{

// Every response starts with a status byte, followed by the payload:
//   Ok    -> the handler's serialised response
//   Error -> a plain-text reason
// This keeps errors part of the transport, so services don't need an error
// message in their own proto schemas.
enum class ResponseStatus : char{
	Ok = 0,
	Error = 1
};

inline std::string makeResponse(ResponseStatus status, const std::string& payload)
{
	std::string response(1, static_cast<char>(status));
	response += payload;
	return response;
}

}

#endif
