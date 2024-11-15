/**
 * @file main.cpp
 * @author jang hun han (janhan@student.42Seoul.kr)
 * @brief IRC main 엔트리 파일
 * @version 0.1
 * @date 2024-10-23
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <iostream>
#include "Server.hpp"

int main(int ac, char** av)
{
	if (ac != 3)
	{
		std::cout << "Usage : ./ircserv [PORT] [PASS]" << std::endl;
		return (1);
	}
	try
	{
		Server server(av[1], av[2]);
		server.Init();
		std::cout << "/connect -nocap 127.0.0.1 " << av[1] << " " << av[2] << std::endl;
		server.Run();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return (1);
	}
	return (0);
}
