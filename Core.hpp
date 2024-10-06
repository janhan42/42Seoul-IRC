#ifndef CORE_HPP
# define CORE_HPP

#include <arpa/inet.h>
#include <string>
#include <iostream>

class Core
{
	public:
		Core(const std::string& port, const std::string& pass);
		~Core();
		void	run();
	private:
		int					mOpt;
		int					mServerSocket;
		const std::string	mPass;
		const std::string	mPort;
		struct	sockaddr_in mServerAddr;
	private:
		Core(const Core&);
		Core& operator=(const Core&);
};

#endif
