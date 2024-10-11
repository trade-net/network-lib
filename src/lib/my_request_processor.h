#include <order.pb.h>
#include <iostream>

class MyRequestProcessor : public RequestProcessor{
public:
	MyRequestProcessor(size_t poolSize): RequestProcessor(poolSize)
	{
		registerHandler<Order, Order>([this](const Order& request){
			return processGetOrder(request);
		});
	}

	Order processGetOrder(const Order& request)
	{
		std::cout << "Input Order: "
			<< "id=" << request.id()
			<< ", name=" << request.name()
			<< ", isBuy=" << request.isbuy() << std::endl;

		Order response;
		response.set_id(request.id() + 1);
		response.set_name("Response order");
		response.set_isbuy(false);

		return response;
	}
};
