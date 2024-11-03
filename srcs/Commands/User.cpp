#include <unistd.h>
#include "../Command.hpp"
#include "../Server.hpp"
#include "../User.hpp"

void Command::User(int fd, std::vector<std::string> commandVec)
{
	/* User <username> <hostname> <servername> <:realname> */
	std::map<int, class User*>& userList = mServer.GetUserList();
	std::map<int, class User*>::iterator it = userList.find(fd);
	if (it->second->GetUserRegist())
	{
		mErrManager.ErrorAlreadyRegistRed462(*it->second);
		return ;
	}
	if (!it->second->GetPassRegist())
	{
		mErrManager.ErrorNotRegistered451(*it->second);
		it->second->AppendUserSendBuf("\r\n");
		send(fd, it->second->GetUserSendBuf().c_str(), it->second->GetUserSendBuf().length(), 0);
		delete it->second;
		userList.erase(fd);
		close(fd);
		return;
	}
	if (commandVec.size() < 5 || !CheckRealName(commandVec[4]))
	{
		mErrManager.ErrorNeedMoreParams461(*it->second);
		it->second->AppendUserSendBuf("/USER <username> <hostname> <servername> <:realname>\r\n");
		return ;
	}
	std::string realname;
	for (size_t i = 4; i < commandVec.size(); i++)
	{
		realname += commandVec[i];
		if (i != commandVec.size() - 1)
			realname += " ";
	}
	it->second->SetUser(commandVec[1], commandVec[2], commandVec[3], realname);
	it->second->SetUserRegist(true);
}

bool Command::CheckRealName(std::string realName)
{
	if (realName.length() == 0)
		return (false);
	if (realName[0] != ':')
		return (false);
	return (true);
}

