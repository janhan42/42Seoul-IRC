#include "./Command.hpp"

Command::Command(Server& server)
: mServer(server)
{}

Command::~Command()
{}

void	Command::Excute(int fd)
{
	std::istringstream	iss(mServer.GetMessage(fd));
	std::string			buffer;
	std::map<int, User>::iterator It;
}
