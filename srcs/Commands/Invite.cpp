#include "../Command.hpp"
#include "../Server.hpp"
#include "../User.hpp"


/*
	- RESPONSE LIST -
	RPL_INVITING (341)
	ERR_NEEDMOREPARAMS (461)
	ERR_NOSUCHCHANNEL (403)
	ERR_NOTONCHANNEL (442)
	ERR_CHANOPRIVSNEEDED (482)
	ERR_USERONCHANNEL (443)
 */
void Command::Invite(int fd, std::vector<std::string> commandVec)
{
	/* INVITE <nickname> <channel> */
	class User*& user = mServer.GetUserList().find(fd)->second;
	if (commandVec.size() < 3)
	{
		mResponse.ErrorNeedMoreParams461(*user, commandVec[1]);
		return;
	}
	class User* target = mServer.FindUser(commandVec[1]);
	// TODO: INVITE 응답중에 401 처리에 대한 얘기가 없음
	if (target == NULL)
	{
		mResponse.ErrorNosuchNick401(*user, commandVec[1]);
		return;
	}
	Channel* channel = mServer.FindChannel(commandVec[2]);
	if (channel == NULL)
	{
		mResponse.ErrorNosuchChannel403(*user, commandVec[2]);
		return;
	}
	if (channel->CheckUserInChannel(user->GetUserFd()) == false)
	{
		mResponse.ErrorNotOnChannel442(*user, commandVec[2]);
		return;
	}
	if (channel->CheckOperator(user->GetUserFd()) == false)
	{
		mResponse.ErrorChanOprivsNeeded482(*user, commandVec[2]);
		return;
	}
	if (channel->CheckUserInChannel(target->GetUserFd()) == true)
	{
		mResponse.ErrorUserOnChannel443(*user, commandVec[1], commandVec[2]);
		return;
	}
	user->AppendUserSendBuf("341 " + user->GetNickName() + " " + commandVec[1] + " " + commandVec[2] + "\r\n");
	target->AppendUserSendBuf(MakeFullName(fd) + " INVITE " + target->GetNickName() + " " + commandVec[2] + "\r\n");
	channel->AppendInviteFdList(target->GetUserFd());
}
