#include "Server.hpp"
#include <fcntl.h>	// for fcntl()
#include <sys/
#include <iostream>

Server::Server(int port, std::string passwd) :
	port(port), passwd(passwd), server_name("sirc")
{
}

int Server::init()
{
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1) return (return_cerr("socket() failed!"));

	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0)
		return (return_cerr("setsockopt"));

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	if (bind(server_fd, reinterpret_cast<struct sockaddr *>(&server_addr),
			 sizeof(server_addr)) == -1)
		return (return_cerr("server bind failed!"));

	return (0);
}

int Server::run()
{
	if (listen(server_fd, SOMAXCONN) == -1)
		return (return_cerr("listen() failed"));
	std::cout << "Server: Waiting for clinet's connection..." << std::endl;
	fcntl(server_fd, F_SETFL, O_NONBLOCK);
	// F_SETFL:		re-set file fd flag with arg
	// O_NONBLOCK:	set fd NONBLOCK

	int kq = kqueue();
	if (kq < 0) }

int return_cerr(const std::string &err_msg)
{
	std::cerr << err_msg << std::endl;
	return (EXIT_FAILURE);
}
