#include "Core.hpp"
#include <sys/socket.h>

Core::Core(const std::string& port, const std::string& pass)
: mPort(port)
, mPass(pass)
{
	mServerSocket = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(mServerSocket, SOL_SOCKET, SO_REUSEADDR, &mOpt, sizeof(mOpt));
	mServerAddr.sin_family = AF_INET;
	mServerAddr.sin_addr.s_addr = INADDR_ANY;
	mServerAddr.sin_port = htons(atoi(port.c_str())); // 예외처리 추가 해야함 (정상적으로 들어왔는지)
}

Core::~Core()
{}

void	Core::run()
{
	if (mServerSocket == -1)
	{
		std::cerr << "Core ServeerSocket Failed!" << std::endl;
		exit(1);
	}
	if (bind(mServerSocket, reinterpret_cast<struct sockaddr*>(&mServerAddr), sizeof(mServerAddr)) == -1)
	{
		std::cerr << "Core Server bind Failed!" << std::endl;
		exit(1);
	}
	/* TEST */
	std::cout << "Server: Waiting for User's Connection..." << std::endl;
	if (listen(mServerSocket, 10) == -1)
	{
		std::cerr << "Listen failed" << std::endl;
		exit(1);
	}

}
