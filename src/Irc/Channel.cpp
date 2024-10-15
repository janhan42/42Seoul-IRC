#include "Channel.hpp"

Channel::Channel(const std::string& channelName, int fd)
{
	memset(this, 0, sizeof(Channel));
	mChannelName = channelName;
	mOperatorFdList.push_back(fd);
	mBot = new Bot();
	mMode = 0;
}

Channel::~Channel()
{
	delete mBot;
}

/* Getter */
std::vector<int> Channel::GetOperatorFdList()
{
	return (mOperatorFdList);
}

std::string Channel::GetChannelName()
{
	return (mChannelName);
}

std::vector<int> Channel::GetUserFdList()
{
	return (mUserFdList);
}

std::string Channel::GetMode()
{
	std::string ret;
	if (mMode & TOPIC)
		ret += "t";
	if (mMode & INVITE)
		ret += "i";
	if (mMode & LIMIT)
		ret += 't';
	if (mMode & KEY)
		ret += "k";
	return (ret);
}

unsigned int Channel::GetLimit()
{
	return (mLimit);
}

Bot* Channel::GetBot()
{
	return (mBot);
}

std::string	Channel::GetTopic()
{
	return (mTopic);
}

/* Setter */
void Channel::SetChannelName(const std::string& channelName)
{
	mChannelName = channelName;
}

void Channel::SetMode(unsigned char mode, char sign)
{
	if (sign == '+')
		mMode |= mode;
	else if (sign == '-')
		mMode &= ~mode;
}

void Channel::SetLimit(unsigned int limit)
{
	mLimit = limit + 1;
}

void Channel::SetTopic(const std::string& topic)
{
	mTopic = topic;
}

void Channel::SetKey(const std::string& key)
{
	mKey = key;
}

/* Check */
bool Channel::CheckMode(unsigned char mode)
{
	if (mMode & mode)
		return (true);
	return (false);
}

bool Channel::CheckInvite(int fd)
{
	if (std::find(mInviteFdList.begin(), mInviteFdList.end(), fd) != mInviteFdList.end())
		return (true);
	return (false);
}

bool Channel::CheckKey(const std::string& key)
{
	if (mKey == key)
		return (true);
	return (false);
}

bool Channel::CheckUserInChannel(int fd)
{
	std::vector<int>::iterator it = FindMyUser(fd);
	if (it != mUserFdList.end())
		return (true);
	return (false);
}

bool Channel::CheckOperator(int fd)
{
	if (std::find(mOperatorFdList.begin(), mOperatorFdList.end(), fd) != mOperatorFdList.end())
		return (true);
	return (false);
}

/* Others */
void Channel::AppendUserFdList(int userFd)
{
	mUserFdList.push_back(userFd);
}

void Channel::RemoveUserFdList(int userFd)
{
	std::vector<int>::iterator it = FindMyUser(userFd);
	if (it != mUserFdList.end())
		mUserFdList.erase(it);
}

std::vector<int>::iterator Channel::FindMyUser(int fd)
{
	return (std::find(mUserFdList.begin(), mUserFdList.end(), fd));
}

void Channel::AddOperatorFd(int fd)
{
	if (std::find(mOperatorFdList.begin(), mOperatorFdList.end(), fd) == mOperatorFdList.end())
		mOperatorFdList.push_back(fd);
}

void Channel::RemoveOperatorFd(int fd)
{
	std::vector<int>::iterator it = std::find(mOperatorFdList.begin(), mOperatorFdList.end(), fd);
	if (it != mOperatorFdList.end())
		mOperatorFdList.erase(it);
}

void Channel::AppendInviteFdList(int fd)
{
	if (std::find(mInviteFdList.begin(), mInviteFdList.end(), fd) == mInviteFdList.end())
		mInviteFdList.push_back(fd);
}
