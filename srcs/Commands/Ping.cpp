#include "../Command.hpp"
#include "../Server.hpp"
#include "../User.hpp"

void Command::Ping(int fd, std::vector<std::string> commandVec)
{
	/* PING <token> */
	std::map<int, class User*>& userList = mServer.GetUserList();
	std::map<int, class User*>::iterator it = userList.find(fd);

	if (commandVec.size() < 2)
	{
		mErrManager.ErrorNeedMoreParams461(*it->second);
		it->second->AppendUserRecvBuf("/PING <token>\r\n");
		return ;
	}
	it->second->AppendUserRecvBuf("PONG " + commandVec[1] + "\r\n");
}
