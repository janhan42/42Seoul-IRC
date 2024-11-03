#include <unistd.h>
#include "../Command.hpp"
#include "../Server.hpp"
#include "../User.hpp"

#ifdef __linux__
#include <sys/epoll.h>	// Linux에서 epoll 사용
#elif defined(__APPLE__)
#include <sys/event.h>	// macOS에서 kqueue 사용
#endif

void Command::Quit(int fd, std::vector<std::string> commandVec)
{
	/* QUIT */
	std::map<int, class User*>&			 userList = mServer.GetUserList();
	std::map<int, class User*>::iterator userIt = userList.find(fd);
	std::vector<std::string> channelList = userIt->second->GetChannelList();
	std::vector<std::string>::iterator channelIt = channelList.begin();
	for (; channelIt != channelList.end(); channelIt++)
	{
		Channel* channel = mServer.FindChannel(*channelIt);
		if (!channel) continue;
		channel->RemoveUserFdList(fd);
		channel->RemoveOperatorFd(fd);
		MsgToAllChannel(fd, channel->GetChannelName(), "QUIT",
						ChannelMessage(1, commandVec));
		if (channel->GetUserFdList().size() == 1)
		{
			mServer.RemoveChannel(channel->GetChannelName());
			delete channel;
		}
	}

#ifdef __linux__
	// Linux epoll 방식
	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLOUT;
	ev.data.fd = userIt->second->GetUserFd();
	epoll_ctl(mServer.GetPollFd(), EPOLL_CTL_DEL, ev.data.fd, &ev);
#elif defined(__APPLE__)
	// macOS kqueue 방식
	struct kevent evSet;
	EV_SET(&evSet, userIt->second->GetUserFd(), EVFILT_READ | EVFILT_WRITE,
		   EV_DELETE, 0, 0, NULL);
	kevent(mServer.GetKqFd(), &evSet, 1, NULL, 0, NULL);
#endif

	mServer.GetMessage(userIt->first) = "";
	delete userIt->second;
	userList.erase(fd);
	close(fd);
}

