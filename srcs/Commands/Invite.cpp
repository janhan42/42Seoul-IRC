#include "../Command.hpp"
#include "../Server.hpp"
#include "../User.hpp"

void Command::Invite(int fd, std::vector<std::string> commandVec)
{
	/* INVITE <nickname> <channel> */
	class User*& user = mServer.GetUserList().find(fd)->second;
	if (commandVec.size() < 3)
	{
		mErrManager.ErrorNeedMoreParams461(*user);
		return;
	}
	std::map<int, class User* >::iterator target = mServer.FindUser(commandVec[1]);
	if (target == mServer.GetUserList().end())
	{
		mErrManager.ErrorNosuchNick401(*user, commandVec[1]);
		return;
	}
	Channel* channel = mServer.FindChannel(commandVec[2]);
	if (channel == NULL)
	{
		mErrManager.ErrorNosuchChannel403(*user, commandVec[2]);
		return;
	}
	if (!channel->CheckUserInChannel(user->GetUserFd()))
	{
		mErrManager.ErrorNotOnChannel442(*user, commandVec[2]);
		return;
	}
	if (!channel->CheckOperator(user->GetUserFd()))
	{
		mErrManager.ErrorChanOprivsNeeded482(*user, commandVec[2]);
		return;
	}
	if (channel->CheckUserInChannel(target->second->GetUserFd()) == true)
	{
		mErrManager.ErrorUserOnChannel443(*user, commandVec[1], commandVec[2]);
		return;
	}
	target->second->AppendUserSendBuf(":" + user->GetNickName() + " INVITE " + target->second->GetNickName() + " " + commandVec[2] + "\r\n");
	user->AppendUserSendBuf("341 :" + user->GetNickName() +  " " + commandVec[1] + " " + commandVec[2] + " :" + RPL_INVITING);
	channel->AppendInviteFdList(target->second->GetUserFd());
}
