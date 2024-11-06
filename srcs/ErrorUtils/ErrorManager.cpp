#include "ErrorManager.hpp"
#include "../User.hpp"

Errormanager::Errormanager()
{}

Errormanager::~Errormanager()
{}


/* No Such Error */
// "401 <client> <nickname> :No such nick/channel"
void Errormanager::ErrorNosuchNick401(User& user, std::string nickName)
{
	user.AppendUserSendBuf("401 " + user.GetNickName() + " " + nickName + " :" + ERR_NOSUCHNICK);
}

// "403 <client> <channel> :No such channel"
void Errormanager::ErrorNosuchChannel403(User& user, std::string channel)
{
	user.AppendUserSendBuf("403 " + user.GetNickName() + " " + channel + " :[" + channel + "] " + ERR_NOSUCHCHANNEL);
}


/* Channel Error */
// "441 <client> <nick> <channel> :They aren't on that channel"
void Errormanager::ErrorUserNotInChannel441(User& user, std::string nickName, std::string channel)
{
	user.AppendUserSendBuf("441 " + user.GetNickName() + " " + nickName + " " + channel + " :" + ERR_USERNOTINCHANNEL);
}

// "442 <client> <channel> :You're not on that channel"
void Errormanager::ErrorNotOnChannel442(User& user, std::string channel)
{
	user.AppendUserSendBuf("442 " + user.GetNickName() + " " + channel + " :" + ERR_NOTONCHANNEL);
}

// "443 <client> <nick> <channel> :is already on channel"
void Errormanager::ErrorUserOnChannel443(User& user, std::string nickName, std::string channel)
{
	user.AppendUserSendBuf("443 " + user.GetNickName() + " " + nickName + " " + channel + " :" + ERR_USERONCHANNEL);
}

// "461 <client> <command> :Not enough parameters"
void Errormanager::ErrorNeedMoreParams461(User& user, std::string command)
{
	user.AppendUserSendBuf("461 " + user.GetNickName() + " " + command + " :" + ERR_NEEDMOREPARAMS);
}

// "162 <client> :You may not reregister"
void Errormanager::ErrorAlreadyRegistRed462(User& user)
{
	user.AppendUserSendBuf("462 " + user.GetNickName() + " :" + ERR_ALREADYREGISTERED);
}

// "451 <client> :You have not registered"
void Errormanager::ErrorNotRegistered451(User& user)
{
	user.AppendUserSendBuf("451: " + user.GetNickName() + " " + ERR_NOTREGISTERED);
}

// "464 <client> :Password incorrect"
void Errormanager::ErrorPasswdMisMatch464(User& user)
{
	user.AppendUserSendBuf("464 " + user.GetNickName() + " :" + ERR_PASSWDMISMATCH);
}

/* mode 47* error */
// "471 <client> <channel> :Cannot join channel (+l)"
void Errormanager::ErrorChannelIsFull471(User& user, std::string channel)
{
	user.AppendUserSendBuf("471 " + user.GetNickName() + " " + channel + " :" + ERR_CHANNELISFULL);
}

// "472 <client> <modechar> :is unknown mode char to me"
void Errormanager::ErrorUnknownMode472(User& user, char mode)
{
	user.AppendUserSendBuf("472 " + user.GetNickName() + " " + mode + " :" + ERR_UNKNOWNMODE);
}

// "473 <client> <channel> :Cannot join channel (+i)"
void Errormanager::ErrorInviteOnlychan473(User& user, std::string channel)
{
	user.AppendUserSendBuf("473 " + user.GetNickName() + " " + channel + " :" + ERR_INVITEONLYCHAN);
}

// "475 <client> <channel> :Cannot join channel (+k)"
void Errormanager::ErrorBadChannelKey475(User& user, std::string channel)
{
	user.AppendUserSendBuf("475 " + user.GetNickName() + " " + channel + " :" + ERR_BADCHANNELKEY);
}

/* mode 48* Error */

// "482 <client> <channel> :You're not channel operator"
void Errormanager::ErrorChanOprivsNeeded482(User& user, std::string channel)
{
	user.AppendUserSendBuf("482 " + user.GetNickName() + " " + channel + " :" + ERR_CHANOPRIVSNEEDED);
}

/* Nick에 있던거 */
// "432 <client> <nick> :Erroneus nickname"
void Errormanager::ErrorErronusNickName432(User& user, std::string name)
{
	user.AppendUserSendBuf("432: " + name + " " + name + " " + ERR_ERRONEUSNICKNAME);
}

// "433 <client> <nick> :Nickname is already in use"
void Errormanager::ErrorNickNameInuse433(User& user, std::string name)
{
	user.AppendUserSendBuf("433: " + name + " " + name + " " + ERR_NICKNAMEINUSE);
}

