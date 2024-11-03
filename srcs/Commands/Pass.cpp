
#include <unistd.h>
#include "../Command.hpp"
#include "../Server.hpp"
#include "../User.hpp"

void Command::Pass(int fd, std::vector<std::string> commandVec)
{
	/* PASS <password> */
	std::map<int, class User*>& userList = mServer.GetUserList();
	std::map<int, class User*>::iterator it = userList.find(fd);
	std::string password = mServer.GetPassWord();
	if (it->second->GetPassRegist())
	{
		mErrManager.ErrorAlreadyRegistRed462(*it->second);
		return ;
	}
	if (commandVec.size() < 2)
	{
		mErrManager.ErrorNeedMoreParams461(*it->second);
		return ;
	}
	if (strcmp(commandVec[1].c_str(), password.c_str()) != 0)
	{
		mErrManager.ErrorPasswdMisMatch464(*it->second);
		send(fd, it->second->GetUserSendBuf().c_str(), it->second->GetUserSendBuf().length(), 0);
		delete it->second;
		userList.erase(fd);
		close(fd);
		return ;
	}
	it->second->SetPassRegist(true);
}
