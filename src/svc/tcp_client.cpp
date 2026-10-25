#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <order.pb.h>
#include <protocol.h>

using boost::asio::ip::tcp;

class Client {
public:
    Client(boost::asio::io_context& io_context, const std::string& server, const std::string& port)
        : io_context_(io_context), socket_(io_context) {
        tcp::resolver resolver(io_context_);
        tcp::resolver::results_type endpoints = resolver.resolve(server, port);
        connect(endpoints);
    }

    void sendMessage(const std::string& message, char requestId = 0) {
        boost::system::error_code ec;

        // Send the message to the server, first byte is the requestId (0 = the Order handler)
        std::string framed(1, requestId);
        framed += message;
        boost::asio::write(socket_, boost::asio::buffer(network::makeFrame(framed)), ec);
        if (!ec) {
            std::cout << "Message sent: " << message << std::endl;
        } else {
            std::cerr << "Error sending message: " << ec.message() << std::endl;
        }

        // Wait for the server's response: a frame header, then exactly the body
        char header[network::kFrameHeaderSize];
        boost::asio::read(socket_, boost::asio::buffer(header), ec);
        if (ec) {
            std::cerr << "Error receiving response: " << ec.message() << std::endl;
            return;
        }

        std::string response(network::readFrameLength(header), '\0');
        boost::asio::read(socket_, boost::asio::buffer(response), ec);
        if (ec || response.empty()) {
            std::cerr << "Error receiving response: " << ec.message() << std::endl;
            return;
        }

        if (response[0] != 0) {
            std::cout << "Error response: " << response.substr(1) << std::endl;
            return;
        }

        Order order;
        order.ParseFromString(response.substr(1));
        std::cout << "Order: "
            << "id=" << order.id()
            << ", name=" << order.name()
            << ", isBuy=" << order.isbuy() << std::endl;
    }

private:
    void connect(const tcp::resolver::results_type& endpoints) {
        boost::system::error_code ec;
        boost::asio::connect(socket_, endpoints, ec);
        if (!ec) {
            std::cout << "Connected to server!" << std::endl;
        } else {
            std::cerr << "Connection failed: " << ec.message() << std::endl;
        }
    }

    boost::asio::io_context& io_context_;
    tcp::socket socket_;
};

int main() {
    try {
        boost::asio::io_context io_context;

        // Create a client and connect to server running on localhost at port 12345
        Client client(io_context, "127.0.0.1", "1234");

        // Send messages and wait for responses
		Order order;
		order.set_id(1);
		order.set_name("Order 1");
		order.set_isbuy(true);

        client.sendMessage(order.SerializeAsString());
        client.sendMessage(order.SerializeAsString());
        client.sendMessage("not a protobuf");   // parse failure -> error response
        client.sendMessage("x", 99);             // unknown request id -> error response
        //client.sendMessage("How are you?");
        //client.sendMessage("How are you?");
        // You can continue sending more messages without closing the connection
        // client.sendMessage("Another message");

    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}

