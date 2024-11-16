#include <unistd.h>
#include "../Command.hpp"
#include "../Server.hpp"
#include "../User.hpp"

/*
	-RESPONSE LIST-
	없음
 */
// 클라이언트가 서버와 연결을 끊는 명령어
// 명령어를 쓴 유저가 접속해있는 채널을 전부 순회하면서
// 해당 채널에서 유저를 지운다.
// 채널에 사람이 남아있다면 누가 나갔다고 메세지를 돌림 
// 이 과정에서 채널에 더이상 유저가 없으면 채널도 같이 삭제
void Command::Quit(int fd, std::vector<std::string> commandVec)
{
	/* QUIT */
	// std::map<int, class User*>& userList = mServer.GetUserList();
	// std::map<int, class User*>::iterator userIt = userList.find(fd);
	class User* user = mServer.FindUser(fd);
	std::vector<std::string> channelList = user->GetChannelList();
	std::vector<std::string>::iterator channelIt = channelList.begin();
	for (; channelIt != channelList.end(); channelIt++)
	{
		Channel* channel = mServer.FindChannel(*channelIt);
		if (!channel)
			continue;
		channel->RemoveUserFdList(fd);
		channel->RemoveOperatorFd(fd);
		//TODO: 유저봇 지울거면 수정필요
		if (channel->GetUserFdList().size() == 1)
		{
			mServer.RemoveChannel(channel->GetChannelName());
			delete channel;
		}
		else // 사람 남아있을때만 보내라고 여기로 옮김
			MsgToAllChannel(fd, channel->GetChannelName(), "QUIT", ChannelMessage(1, commandVec));
	}
	// struct kevent evSet;
	// EV_SET(&evSet, userIt->second->GetUserFd(), EVFILT_READ | EVFILT_WRITE, EV_DELETE, 0, 0,NULL);
	// kevent(mServer.GetKqFd(), &evSet, 1, NULL, 0, NULL);
	// mServer.GetMessage(userIt->first) = "";
	// delete userIt->second;
	// userList.erase(fd);
	// close(fd);
	mServer.DeleteUserFromServer(fd);
}
