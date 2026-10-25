#include <gtest/gtest.h>

#include <tcp_async_server.h>
#include <tcp_client.h>
#include <protocol.h>
#include <request_processor.h>
#include <order.pb.h>

#include <boost/asio.hpp>
#include <chrono>
#include <memory>
#include <string>
#include <sys/socket.h>
#include <thread>

namespace {

using boost::asio::ip::tcp;

// Request id 0: takes an Order, answers with an Order whose id is one higher.
class EchoProcessor : public network::RequestProcessor{
public:
	EchoProcessor()
	{
		registerHandler<Order, Order>([](const Order& request){
			Order response;
			response.set_id(request.id() + 1);
			response.set_name("response");
			return response;
		});
	}
};

// Runs a real server on an ephemeral port for the duration of a test.
class FramingTest : public ::testing::Test{
protected:
	void SetUp() override
	{
		server = std::make_unique<network::TcpAsyncServer>(processor, /*numLoops=*/2, /*port=*/0);
		serverThread = std::thread([this]{ server->run(); });
	}

	void TearDown() override
	{
		server->stop();
		serverThread.join();
	}

	// a plain socket with a receive timeout, so a server bug fails the test instead of hanging it
	tcp::socket connectRaw()
	{
		tcp::socket socket(io);
		socket.connect(tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), server->port()));
		timeval timeout{3, 0};
		setsockopt(socket.native_handle(), SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
		return socket;
	}

	static std::string requestFrame(int id, int requestId = 0, size_t nameLength = 0)
	{
		Order order;
		order.set_id(id);
		order.set_name(std::string(nameLength, 'x'));
		std::string body(1, static_cast<char>(requestId));
		body += order.SerializeAsString();
		return network::makeFrame(body);
	}

	// reads one response frame and returns its body ([status][payload]); empty on EOF/timeout
	static std::string readFrame(tcp::socket& socket)
	{
		boost::system::error_code ec;
		char header[network::kFrameHeaderSize];
		boost::asio::read(socket, boost::asio::buffer(header), ec);
		if(ec) return "";

		std::string body(network::readFrameLength(header), '\0');
		boost::asio::read(socket, boost::asio::buffer(body), ec);
		return ec ? "" : body;
	}

	static int responseOrderId(const std::string& body)
	{
		EXPECT_FALSE(body.empty());
		EXPECT_EQ(body[0], static_cast<char>(network::ResponseStatus::Ok));
		Order order;
		EXPECT_TRUE(order.ParseFromString(body.substr(1)));
		return order.id();
	}

	// true once the server has closed the connection
	static bool closedByServer(tcp::socket& socket)
	{
		boost::system::error_code ec;
		char byte;
		socket.read_some(boost::asio::buffer(&byte, 1), ec);
		return ec == boost::asio::error::eof || ec == boost::asio::error::connection_reset;
	}

	EchoProcessor processor;
	boost::asio::io_context io;
	std::unique_ptr<network::TcpAsyncServer> server;
	std::thread serverThread;
};

} // namespace

TEST_F(FramingTest, SingleRequestGetsOneResponse) {
	auto socket = connectRaw();
	boost::asio::write(socket, boost::asio::buffer(requestFrame(10)));

	EXPECT_EQ(responseOrderId(readFrame(socket)), 11);
}

TEST_F(FramingTest, TwoRequestsInOneWriteAreBothProcessed) {
	auto socket = connectRaw();
	boost::asio::write(socket, boost::asio::buffer(requestFrame(1) + requestFrame(2)));

	EXPECT_EQ(responseOrderId(readFrame(socket)), 2);
	EXPECT_EQ(responseOrderId(readFrame(socket)), 3);
}

TEST_F(FramingTest, ManyPipelinedRequestsKeepTheirOrder) {
	auto socket = connectRaw();
	std::string burst;
	for(int i = 0; i < 200; ++i) burst += requestFrame(i);
	boost::asio::write(socket, boost::asio::buffer(burst));

	for(int i = 0; i < 200; ++i){
		ASSERT_EQ(responseOrderId(readFrame(socket)), i + 1);
	}
}

TEST_F(FramingTest, RequestSplitAcrossWritesIsReassembled) {
	auto socket = connectRaw();
	const std::string frame = requestFrame(7);
	const size_t half = frame.size() / 2;

	boost::asio::write(socket, boost::asio::buffer(frame.data(), half));
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	boost::asio::write(socket, boost::asio::buffer(frame.data() + half, frame.size() - half));

	EXPECT_EQ(responseOrderId(readFrame(socket)), 8);
}

TEST_F(FramingTest, HeaderSplitAcrossWritesIsReassembled) {
	auto socket = connectRaw();
	const std::string frame = requestFrame(20);

	boost::asio::write(socket, boost::asio::buffer(frame.data(), 2)); // half of the length field
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	boost::asio::write(socket, boost::asio::buffer(frame.data() + 2, frame.size() - 2));

	EXPECT_EQ(responseOrderId(readFrame(socket)), 21);
}

TEST_F(FramingTest, RequestSentOneByteAtATime) {
	auto socket = connectRaw();
	socket.set_option(tcp::no_delay(true));
	const std::string frame = requestFrame(30) + requestFrame(31);

	for(char byte : frame){
		boost::asio::write(socket, boost::asio::buffer(&byte, 1));
	}

	EXPECT_EQ(responseOrderId(readFrame(socket)), 31);
	EXPECT_EQ(responseOrderId(readFrame(socket)), 32);
}

TEST_F(FramingTest, RequestLargerThanTheReadBufferIsAccepted) {
	auto socket = connectRaw();
	boost::asio::write(socket, boost::asio::buffer(requestFrame(5, 0, /*nameLength=*/5000)));

	EXPECT_EQ(responseOrderId(readFrame(socket)), 6);
}

TEST_F(FramingTest, UnknownRequestIdGetsAnErrorAndTheSessionSurvives) {
	auto socket = connectRaw();
	boost::asio::write(socket, boost::asio::buffer(requestFrame(1, /*requestId=*/99)));

	std::string error = readFrame(socket);
	ASSERT_FALSE(error.empty());
	EXPECT_EQ(error[0], static_cast<char>(network::ResponseStatus::Error));

	boost::asio::write(socket, boost::asio::buffer(requestFrame(1)));
	EXPECT_EQ(responseOrderId(readFrame(socket)), 2);
}

TEST_F(FramingTest, OversizedFrameClosesTheConnection) {
	auto socket = connectRaw();
	const char header[] = {0x7F, 0x00, 0x00, 0x00}; // ~2 GB
	boost::asio::write(socket, boost::asio::buffer(header));

	EXPECT_TRUE(closedByServer(socket));
}

TEST_F(FramingTest, ZeroLengthFrameClosesTheConnection) {
	auto socket = connectRaw();
	const char header[] = {0x00, 0x00, 0x00, 0x00};
	boost::asio::write(socket, boost::asio::buffer(header));

	EXPECT_TRUE(closedByServer(socket));
}

TEST_F(FramingTest, CloseRequestIsAcknowledgedThenTheServerCloses) {
	auto socket = connectRaw();
	boost::asio::write(socket, boost::asio::buffer(network::makeFrame(std::string(1, static_cast<char>(network::kCloseRequestId)))));

	std::string reply = readFrame(socket);
	ASSERT_EQ(reply.size(), 2u);
	EXPECT_EQ(reply[0], static_cast<char>(network::ResponseStatus::Ok));
	EXPECT_EQ(reply[1], static_cast<char>(network::kCloseRequestId));
	EXPECT_TRUE(closedByServer(socket));
}

TEST_F(FramingTest, TcpClientRoundTripsAndSurfacesErrors) {
	network::TcpClient client("127.0.0.1", std::to_string(server->port()));

	Order order;
	order.set_id(41);
	Order response;
	ASSERT_TRUE(response.ParseFromString(client.send(0, order.SerializeAsString())));
	EXPECT_EQ(response.id(), 42);

	EXPECT_THROW(client.send(99, "anything"), network::RequestError);

	// still usable after an error response
	ASSERT_TRUE(response.ParseFromString(client.send(0, order.SerializeAsString())));
	EXPECT_EQ(response.id(), 42);
}
