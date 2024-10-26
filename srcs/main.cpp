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
	(void)av;
	if (ac != 3)
	{
		std::cout << "Usage : ./ircserv [PORT] [PASS]" << std::endl;
		return (1);
	}
	try
	{
		Server server("1577", "1234");
		server.Init();
		server.Run();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return (1);
	}
}