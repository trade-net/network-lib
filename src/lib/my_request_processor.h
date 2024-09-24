#include <order.h>
#include <iostream>

class MyRequestProcessor : public RequestProcessor{
public:
	MyRequestProcessor()
	{
		registerHandler<Order, Order>("getOrder", [this](const Order& request){
			return processGetOrder(request);
		});
	}

	Order processGetOrder(const Order& request)
	{
		std::cout << "Input Order: "
			<< "id=" << request.id
			<< ", name=" << request.name 
			<< ", isBuy=" << request.isbuy << std::endl;

		Order response;
		response.id = request.id + 1;
		response.name = "Response order";
		response.isbuy = false;

		return response;
	}
};
