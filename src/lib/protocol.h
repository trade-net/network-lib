#ifndef INCLUDED_PROTOCOL
#define INCLUDED_PROTOCOL

#include <cstddef>
#include <cstdint>
#include <string>

namespace network{

// TCP is a byte stream, so every message in either direction is wrapped in a frame:
//   [4-byte big-endian body length][body]
// Request body:  [1-byte request id][request payload]
// Response body: [1-byte status][response payload]   (see below)
constexpr size_t kFrameHeaderSize = 4;
constexpr uint32_t kMaxFrameSize = 64 * 1024; // largest body either side will accept

// Request id that asks the server to end the session after replying.
constexpr int kCloseRequestId = 127;

inline std::string makeFrame(const std::string& body)
{
	const uint32_t length = static_cast<uint32_t>(body.size());
	std::string frame;
	frame.reserve(kFrameHeaderSize + body.size());
	frame.push_back(static_cast<char>((length >> 24) & 0xFF));
	frame.push_back(static_cast<char>((length >> 16) & 0xFF));
	frame.push_back(static_cast<char>((length >> 8) & 0xFF));
	frame.push_back(static_cast<char>(length & 0xFF));
	frame += body;
	return frame;
}

// header must point to at least kFrameHeaderSize bytes
inline uint32_t readFrameLength(const char* header)
{
	return (static_cast<uint32_t>(static_cast<unsigned char>(header[0])) << 24)
		| (static_cast<uint32_t>(static_cast<unsigned char>(header[1])) << 16)
		| (static_cast<uint32_t>(static_cast<unsigned char>(header[2])) << 8)
		| static_cast<uint32_t>(static_cast<unsigned char>(header[3]));
}

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
