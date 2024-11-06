#include "../Command.hpp"
#include "../Server.hpp"
#include "../Utils/Split.hpp"
#include "../User.hpp"
#include "../Bot.hpp"

/*
	- RESPONSE LIST-
	ERR_NEEDMOREPARAMS (461)
	ERR_NOSUCHCHANNEL (403)
	ERR_TOOMANYCHANNELS (405) -> 서버에서 최대 채널 수 제한을 안해뒀기 때문에 비사용
	ERR_BADCHANNELKEY (475)
	ERR_BANNEDFROMCHAN (474) -> b 모드 지원안하기 때문에 비사용
	ERR_CHANNELISFULL (471)
	ERR_INVITEONLYCHAN (473)
	ERR_BADCHANMASK (476) -> ERR_NOSUCHCHANNEL로 대체
	RPL_TOPIC (332)
	RPL_TOPICWHOTIME (333)
	RPL_NAMREPLY (353)
	RPL_ENDOF (366)
 */
void Command::Join(int fd, std::vector<std::string> commandVec)
{
	/* JOIN <channel>{,<channel>} [<key>{,<key>}] */
	/*
		commandVec[0] = JOIN
		commandVec[1] = <channel>{,<channel>}
		commnadVec[2] = [<key>{,key>}]
	*/
	if (commandVec.size() < 2)
	{
		mErrManager.ErrorNeedMoreParams461(*mServer.GetUserList().find(fd)->second, commandVec[1]);
		return;
	}
	std::vector<std::string> joinChannel = split(commandVec[1], ',');
	std::vector<std::string>::iterator iter = joinChannel.begin();
	std::vector<std::string> joinKey;
	std::vector<std::string>::iterator keyIt;
	if (commandVec.size() > 2)
	{
		joinKey = split(commandVec[2], ',');
		keyIt = joinKey.begin();
	}
	std::map<int, class User*>& userList = mServer.GetUserList();
	class User*& user = userList.find(fd)->second;
	while(iter != joinChannel.end())
	{
		if ((*iter)[0] != '#' && (*iter)[0] != '&')
		{
			mErrManager.ErrorNosuchChannel403(*user, *iter);
			iter++;
			if (commandVec.size() > 2 || keyIt != joinKey.end())
				keyIt++;
			continue;
		}
		std::map<std::string, Channel*>& channelList = mServer.GetChannelList();
		std::map<std::string, Channel*>::iterator channelIt = channelList.find(*iter);
		if (channelIt != channelList.end()) // channel exists
		{
			/* conditions: if users can join to channel */
			Channel* channel = channelIt->second;
			if (channel->CheckUserInChannel(fd)) // aready in channel
			{
				iter++;
				if (commandVec.size() > 2 || keyIt != joinKey.end())
					keyIt++;
				continue;
			}
			if (channel->CheckMode(INVITE) == true) // MODE(invite) == true;
			{
				if (channel->CheckInvite(fd) == false)
				{
					mErrManager.ErrorInviteOnlychan473(*user, *iter);
					iter++;
					if (commandVec.size() > 2 || keyIt != joinKey.end())
						keyIt++;
					continue;
				}
			}
			if (channel->CheckMode(KEY) == true) // MODE(key) == true
			{
				if (commandVec.size() <= 2 || keyIt == joinKey.end() || channel->CheckKey(*keyIt) == false) // Bad Key
				{
					mErrManager.ErrorBadChannelKey475(*user, *iter);
					iter++;
					if (commandVec.size() > 2 || keyIt != joinKey.end())
						keyIt++;
					continue;
				}
			}
			if (channel->CheckMode(LIMIT) == true) // MODE(limit) == true
			{
				if (channel->GetUserFdList().size() >= channel->GetLimit())
				{
					mErrManager.ErrorChannelIsFull471(*user, *iter);
					iter++;
					if (commandVec.size() > 2 || keyIt != joinKey.end())
						keyIt++;
					continue;
				}
			}
			std::string channelName = channelIt->second->GetChannelName();
			channelIt->second->AppendUserFdList(fd);
			user->AppendChannelList(channelName);							// join user to channel
			MsgToAllChannel(fd, channelName, "JOIN", "");	// send join-message to users(in channel)
			TopicMsg(fd, channelName);
		}
		else // channel not exicsts (new channel)
		{
			mServer.AppendNewChannel(*iter, fd);					// create new channel
			mServer.FindChannel(*iter)->AppendUserFdList(-1);	// join user to channel
			mServer.FindChannel(*iter)->AppendUserFdList(fd);		// join user to channel
			user->AppendChannelList(*iter);
			MsgToAllChannel(fd, *iter, "JOIN", "");
			mServer.FindChannel(*iter)->AddOperatorFd(fd);
		}
		NameListMsg(fd, *iter); // 353 RPL_NAMREPLAY, 366 RPL_ENDOFNAMES
		MsgToAllChannel(-1, *iter, "PRIVMSG", mServer.FindChannel(*iter)->GetBot()->Introduce());
		iter++;
		if (commandVec.size() > 2 || keyIt != joinKey.end())
			keyIt++;
	}
}

void Command::TopicMsg(int fd, std::string channelName)
{
	std::map<std::string, Channel* >& channelList = mServer.GetChannelList();
	Channel* channel = channelList.find(channelName)->second;
	std::string topic = channel->GetTopic();

	if (topic.length() == 0)
		return;

	topic = topic.substr(1, topic.length() - 1);
	class User*& user = mServer.GetUserList().find(fd)->second;
	user->AppendUserSendBuf("332 " + user->GetNickName() + " " + channelName + " :" + topic + "\r\n");
}
