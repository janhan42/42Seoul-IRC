#include "../Command.hpp"
#include "../Server.hpp"
#include "../User.hpp"

/*
	- RESPONSE LIST -
	ERR_NEEDMOREPARAMS (461)
	ERR_NOORIGIN (409)
 */
void Command::Ping(int fd, std::vector<std::string> commandVec)
{
	/* PING <token> */
	std::map<int, class User*>& userList = mServer.GetUserList();
	std::map<int, class User*>::iterator it = userList.find(fd);

	if (commandVec.size() < 2)
	{
		mResponse.ErrorNeedMoreParams461(*it->second, commandVec[1]);
		it->second->AppendUserSendBuf("/PING <token>\r\n");
		return ;
	}
	it->second->AppendUserSendBuf("PONG " + commandVec[1] + " :" + commandVec[1] + "\r\n");
}
