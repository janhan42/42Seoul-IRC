/**
 * @file main.cpp
 * @author jang hun han (janhan@student.42Seoul.kr)
 * @brief
 * @version 0.1
 * @date 2024-10-10
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "common.hpp"
#include "./src/server/Server.hpp"

/**
 * @brief
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(const int argc, const char** argv)
{
	// argv[1] : PORT argv[2] : PASSWORD
	if (argc != 3
		|| isInt(argv[1]) == false
		|| std::atoi(argv[1]) >> 16 != 0 // port는 16bits의 형태로 저장하기 때문에
		|| std::string(argv[2]).empty())
	{
		std::cerr << "ERROR: Usage: ./irc-serv [PORT] [PASS]" << std::endl;
		return EXIT_FAILURE;
	}
	 // server의 포트 번호 지정을 할때 htons를 사용 하기 때문에 int 형으로 저장
	const int port = std::atoi(argv[1]);
	const std::string password(argv[2]);

	{
		Server server(port, password);
		server.Init();
		server.Run();
	}
	return EXIT_SUCCESS;
}
