#include <string>
#include "./srcs/Server.hpp"

// server class 만들기 전까지만

int main(void)
{
	int			port = 6667;
	std::string passwd = "1234";

	// input_check(port, passwd);

	Server server(port, passwd);
	if (server.init() == 0) server.run();

	return (0);
}
