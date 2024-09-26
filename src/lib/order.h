#include <string>
#include <iostream>

struct Order{
	int id;
	std::string name;
	bool isbuy;

	bool ParseFromString(const std::string& data)
	{
		std::cout << "parsing data: " << data << std::endl;
		id = 1;
		name = "Order 1";
		isbuy = true;
		return true;
	}

	std::string SerializeAsString()
	{
		std::cout << "Serialising Order: "
			<< "id=" << id
			<< ", name=" << name 
			<< ", isBuy=" << isbuy << std::endl;
		return "This is the serialised output";
	}
};
