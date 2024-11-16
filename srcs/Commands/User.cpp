#include <unistd.h>
#include "../Command.hpp"
#include "../Server.hpp"
#include "../User.hpp"

void Command::User(int fd, std::vector<std::string> commandVec)
{
	/* User <username> <hostname> <servername> <:realname> */
	// std::map<int, class User*>& userList = mServer.GetUserList();
	// std::map<int, class User*>::iterator it = userList.find(fd);
	// class User* user = it->second;
	class User* user = mServer.FindUser(fd);
	if (user->GetUserRegist())
	{
		mResponse.ErrorAlreadyRegistRed462(*user);
		return ;
	}
	if (!user->GetPassRegist())
	{
		mResponse.ErrorNotRegistered451(*user);
		send(fd, user->GetUserSendBuf().c_str(), user->GetUserSendBuf().length(), 0);
		// delete it->second;
		// userList.erase(fd);
		// close(fd);
		mServer.DeleteUserFromServer(fd);
		return;
	}
	if (commandVec.size() < 5 || !CheckRealName(commandVec[4]))
	{
		mResponse.ErrorNeedMoreParams461(*user, commandVec[1]);
		user->AppendUserSendBuf("/USER <username> <hostname> <servername> <:realname>\r\n");
		return ;
	}
	std::string realname;
	for (size_t i = 4; i < commandVec.size(); i++)
	{
		realname += commandVec[i];
		if (i != commandVec.size() - 1)
			realname += " ";
	}
	user->SetUser(commandVec[1], commandVec[2], commandVec[3], realname);
	user->SetUserRegist(true);
}

bool Command::CheckRealName(std::string realName)
{
	if (realName.length() == 0)
		return (false);
	if (realName[0] != ':')
		return (false);
	return (true);
}

