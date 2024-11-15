#include "ResponseManager.hpp"
#include "../User.hpp"
#include "../Command.hpp"

ResponseManager::ResponseManager()
{}

ResponseManager::~ResponseManager()
{}


void ResponseManager::RPLWelcome001(User& user, std::string FullName)
{
	user.AppendUserSendBuf(":SIRC 001 " + user.GetNickName() + " :Welcome to the Smoking Relay Chat " + FullName + " \r\n");
}

void ResponseManager::RPLYourHost002(User& user)
{
	user.AppendUserSendBuf(":SIRC 002 " + user.GetNickName() + " :Your host is SIRC, running version v1.0.0 \r\n");
}

void ResponseManager::RPLCreated003(User& user)
{
	user.AppendUserSendBuf(":SIRC 003 " + user.GetNickName() + " :This server was created 2024 \r\n");
}

void ResponseManager::RPLMyInfo004(User& user)
{
	user.AppendUserSendBuf(":SIRC 004  " + user.GetNickName() + " :SIRC v1.0.0 \r\n");
}

void ResponseManager::RPLISupport005(User& user)
{
	user.AppendUserSendBuf(":SIRC 005 " + user.GetNickName() + " :MAXNICKLEN=9 MAXCHANNELLEN=200 :are supported by this server\r\n");
}

void ResponseManager::ErrorNoTopic331(User& user, std::string channel)
{
	user.AppendUserSendBuf("331 " + user.GetNickName() + " " + channel + " :" + RPL_NOTOPIC);
}

void ResponseManager::RPL_NamReply353(User& user, std::string channel, std::string meassage)
{
	user.AppendUserSendBuf("353 " + user.GetNickName() + " = " + channel + " :" + meassage + "\r\n");
}

void ResponseManager::RPL_EndOfNames366(User& user, std::string channel)
{
	user.AppendUserSendBuf("366 " + user.GetNickName() + " " + channel + " " + RPL_ENDOFNAMES);
}

/* No Such Error */
// "401 <client> <nickname> :No such nick/channel"
void ResponseManager::ErrorNosuchNick401(User& user, std::string nickName)
{
	user.AppendUserSendBuf("401 " + user.GetNickName() + " " + nickName + " :" + ERR_NOSUCHNICK);
}

// "403 <client> <channel> :No such channel"
void ResponseManager::ErrorNosuchChannel403(User& user, std::string channel)
{
	user.AppendUserSendBuf("403 " + user.GetNickName() + " " + channel + " :[" + channel + "] " + ERR_NOSUCHCHANNEL);
}

void ResponseManager::ErrorNoTextToSend412(User& user)
{
	user.AppendUserSendBuf("412 " + user.GetNickName() + " :" + ERR_NOTEXTTOSEND);
}

void ResponseManager::RPLNoMotd422(User& user)
{
	user.AppendUserSendBuf(":SIRC 422  " + user.GetNickName() + " :MOTD File is missing\r\n");
}

void ResponseManager::ErrorNoNickNameGiven431(User& user)
{
	user.AppendUserSendBuf("431 " + user.GetNickName() + " :" + ERR_NONICKNAMEGIVEN);
}

/* Channel Error */
// "441 <client> <nick> <channel> :They aren't on that channel"
void ResponseManager::ErrorUserNotInChannel441(User& user, std::string nickName, std::string channel)
{
	user.AppendUserSendBuf("441 " + user.GetNickName() + " " + nickName + " " + channel + " :" + ERR_USERNOTINCHANNEL);
}

// "442 <client> <channel> :You're not on that channel"
void ResponseManager::ErrorNotOnChannel442(User& user, std::string channel)
{
	user.AppendUserSendBuf("442 " + user.GetNickName() + " " + channel + " :" + ERR_NOTONCHANNEL);
}

// "443 <client> <nick> <channel> :is already on channel"
void ResponseManager::ErrorUserOnChannel443(User& user, std::string nickName, std::string channel)
{
	user.AppendUserSendBuf("443 " + user.GetNickName() + " " + nickName + " " + channel + " :" + ERR_USERONCHANNEL);
}

// "461 <client> <command> :Not enough parameters"
void ResponseManager::ErrorNeedMoreParams461(User& user, std::string command)
{
	user.AppendUserSendBuf("461 " + user.GetNickName() + " " + command + " :" + ERR_NEEDMOREPARAMS);
}

// "162 <client> :You may not reregister"
void ResponseManager::ErrorAlreadyRegistRed462(User& user)
{
	user.AppendUserSendBuf("462 " + user.GetNickName() + " :" + ERR_ALREADYREGISTERED);
}

// "451 <client> :You have not registered"
void ResponseManager::ErrorNotRegistered451(User& user)
{
	user.AppendUserSendBuf("451: " + user.GetNickName() + " " + ERR_NOTREGISTERED);
}

// "464 <client> :Password incorrect"
void ResponseManager::ErrorPasswdMisMatch464(User& user)
{
	user.AppendUserSendBuf("464 " + user.GetNickName() + " :" + ERR_PASSWDMISMATCH);
}

/* mode 47* error */
// "471 <client> <channel> :Cannot join channel (+l)"
void ResponseManager::ErrorChannelIsFull471(User& user, std::string channel)
{
	user.AppendUserSendBuf("471 " + user.GetNickName() + " " + channel + " :" + ERR_CHANNELISFULL);
}

// "472 <client> <modechar> :is unknown mode char to me"
void ResponseManager::ErrorUnknownMode472(User& user, char mode)
{
	user.AppendUserSendBuf("472 " + user.GetNickName() + " " + mode + " :" + ERR_UNKNOWNMODE);
}

// "473 <client> <channel> :Cannot join channel (+i)"
void ResponseManager::ErrorInviteOnlychan473(User& user, std::string channel)
{
	user.AppendUserSendBuf("473 " + user.GetNickName() + " " + channel + " :" + ERR_INVITEONLYCHAN);
}

// "475 <client> <channel> :Cannot join channel (+k)"
void ResponseManager::ErrorBadChannelKey475(User& user, std::string channel)
{
	user.AppendUserSendBuf("475 " + user.GetNickName() + " " + channel + " :" + ERR_BADCHANNELKEY);
}

/* mode 48* Error */

// "482 <client> <channel> :You're not channel operator"
void ResponseManager::ErrorChanOprivsNeeded482(User& user, std::string channel)
{
	user.AppendUserSendBuf("482 " + user.GetNickName() + " " + channel + " :" + ERR_CHANOPRIVSNEEDED);
}

/* Nick에 있던거 */
// "432 <client> <nick> :Erroneus nickname"
void ResponseManager::ErrorErronusNickName432(User& user, std::string name)
{
	user.AppendUserSendBuf("432: " + name + " " + name + " " + ERR_ERRONEUSNICKNAME);
}

// "433 <client> <nick> :Nickname is already in use"
void ResponseManager::ErrorNickNameInuse433(User& user, std::string name)
{
	user.AppendUserSendBuf("433: " + name + " " + name + " " + ERR_NICKNAMEINUSE);
}

void ResponseManager::ErrorUsersDontMatch502(User& user)
{
	user.AppendUserSendBuf("502 " + user.GetNickName() + " :" + ERR_USERSDONTMATCH);
}
