
#include <unistd.h>
#include "../Command.hpp"
#include "../Server.hpp"
#include "../User.hpp"

/*
	- RESPONSE LIST -
	ERR_NEEDMOREPARAMS (461)
	ERR_ALREADYREGISTERED (462)
	ERR_PASSWDMISMATCH (464)
 */
void Command::Pass(int fd, std::vector<std::string> commandVec)
{
	/* PASS <password> */
	// std::map<int, class User*>& userList = mServer.GetUserList();
	// std::map<int, class User*>::iterator it = userList.find(fd);

	class User* user = mServer.FindUser(fd);
	std::string password = mServer.GetPassWord();
	if (user->GetPassRegist())
	{
		mResponse.ErrorAlreadyRegistRed462(*user);
		return ;
	}
	if (commandVec.size() < 2)
	{
		mResponse.ErrorNeedMoreParams461(*user, commandVec[1]);
		return ;
	}
	//if (strcmp(commandVec[1].c_str(), password.c_str()) != 0)
	if (commandVec[1] != password)
	{
		mResponse.ErrorPasswdMisMatch464(*user);
		send(fd, user->GetUserSendBuf().c_str(), user->GetUserSendBuf().length(), 0);
		// delete it->second;
		// userList.erase(fd);
		// close(fd);
		mServer.DeleteUserFromServer(fd);
		return ;
	}
	user->SetPassRegist(true);
}
