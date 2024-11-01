#include <unistd.h>
#include "../Command.hpp"
#include "../Server.hpp"
#include "../User.hpp"

void Command::Quit(int fd, std::vector<std::string> commandVec)
{
	/* QUIT */
	std::map<int, class User*> &userList = mServer.GetUserList();
	std::map<int, class User*>::iterator userIt = userList.find(fd);
	std::vector<std::string> channelList = userIt->second->GetChannelList();
	std::vector<std::string>::iterator channelIt = channelList.begin();
	for (; channelIt != channelList.end(); channelIt++)
	{
		Channel* channel = mServer.FindChannel(*channelIt);
		if (!channel)
			continue;
		channel->RemoveUserFdList(fd);
		channel->RemoveOperatorFd(fd);
		MsgToAllChannel(fd, channel->GetChannelName(), "QUIT", ChannelMessage(1, commandVec));
		if (channel->GetUserFdList().size() == 1)
		{
			mServer.RemoveChannel(channel->GetChannelName());
			delete channel;
		}
	}
	struct kevent evSet;
	EV_SET(&evSet, userIt->second->GetUserFd(), EVFILT_READ | EVFILT_WRITE, EV_DELETE, 0, 0,NULL);
	kevent(mServer.GetKqFd(), &evSet, 1, NULL, 0, NULL);
	userIt->second->ClearUser();
	mServer.GetMessage(userIt->first) = "";
	delete userIt->second;
	userList.erase(fd);
	close(fd);
}
