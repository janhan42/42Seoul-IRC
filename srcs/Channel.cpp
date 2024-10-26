#include "Channel.hpp"
#include "Bot.hpp"

Channel::Channel(const std::string &channelName, int fd)
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
std::string	Channel::GetChannelName(void)
{
	return (mChannelName);
}

std::vector<int>	Channel::GetUserFdList(void)
{
	return (mUserFdList);
}

Bot*	Channel::GetBot()
{
	return (mBot);
}

std::vector<int>	Channel::GetOperatorFdList(void)
{
	return (mOperatorFdList);
}

std::string	Channel::GetMode(void)
{
	std::string ret;
	if (mMode & TOPIC)
		ret += "t";
	if (mMode & INVITE)
		ret += "i";
	if (mMode & LIMIT)
		ret += "l";
	if (mMode & KEY)
		ret += "k";
	return (ret);
}

unsigned int	Channel::GetLimit(void)
{
	return (mLimit);
}

std::string	Channel::GetTopic(void)
{
	return (mTopic);
}


/* Setter */
void	Channel::SetChannelName(std::string channelName)
{
	mChannelName = channelName;
}

void	Channel::SetMode(unsigned char mode, char sign)
{
	if (sign == '+')
		mMode |= mode;
	else if (sign == '-')
		mMode &= ~mode;
}

void	Channel::SetLimit(unsigned int num)
{
	mLimit = num + 1;
}

void	Channel::SetTopic(std::string topic)
{
	mTopic = topic;
}

void	Channel::SetKey(std::string key)
{
	mKey = key;
}


/* Check */
bool	Channel::CheckMode(unsigned char mode)
{
	if (mMode & mode)
		return (true);
	return (false);
}

bool	Channel::CheckInvite(int fd)
{
	if (std::find(mInviteFdList.begin(), mInviteFdList.end(), fd) != mInviteFdList.end())
		return (true);
	return (false);
}

bool	Channel::CheckKey(std::string key)
{
	if (mKey == key)
		return (true);
	return (false);
}

bool	Channel::CheckUserInChannel(int fd)
{
	std::vector<int>::iterator it = FindMyUserIt(fd);
	if (it != mUserFdList.end())
		return (true);
	return (false);
}

bool	Channel::CheckOperator(int fd)
{
	if (std::find(mOperatorFdList.begin(), mOperatorFdList.end(), fd) != mOperatorFdList.end())
		return (true);
	return (false);
}


/* Others */
void	Channel::AppendUserFdList(int fd)
{
	mUserFdList.push_back(fd);
}

void	Channel::RemoveUserFdList(int fd)
{
	std::vector<int>::iterator it = FindMyUserIt(fd);
	if (it != mUserFdList.end())
		mUserFdList.erase(it);
}

std::vector<int>::iterator	Channel::FindMyUserIt(int fd)
{
	return (std::find(mUserFdList.begin(), mUserFdList.end(), fd));
}

void	Channel::AddOperatorFd(int fd)
{
	if (std::find(mOperatorFdList.begin(), mOperatorFdList.end(), fd) == mOperatorFdList.end())
		mOperatorFdList.push_back(fd);
}

void	Channel::RemoveOperatorFd(int fd)
{
	std::vector<int>::iterator it = std::find(mOperatorFdList.begin(), mOperatorFdList.end(), fd);
	if (it != mOperatorFdList.end())
		mOperatorFdList.erase(it);
}

void	Channel::AppendInviteFdList(int fd)
{
	if (std::find(mInviteFdList.begin(), mInviteFdList.end(), fd) == mInviteFdList.end())
		mInviteFdList.push_back(fd);
}
