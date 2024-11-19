#include "Channel.hpp"
#include "Bot.hpp"

/* Constructor & Destructor */
/**
 * @brief
 * Channel 클래스의 생성자
 * @param channelName 생성할 채널의 이름
 * @param fd 채널 생성 시 설정할 초기 운영자 파일 디스크립터
 */
Channel::Channel(const std::string &channelName, int fd)
{
	memset(this, 0, sizeof(Channel));
	mChannelName = channelName;
	mOperatorFdList.push_back(fd);
	mBot = new Bot();
	mMode = 0;
}

/**
 * @brief
 * Channel 클래스의 소멸자
 * 동적으로 할당된 Bot 객체를 해제합니다.
 */
Channel::~Channel()
{
	delete mBot;
}

/* Getter */
/**
 * @brief
 * 채널 이름을 반환하는 함수
 * @return std::string 채널 이름
 */
std::string	Channel::GetChannelName(void)
{
	return (mChannelName);
}

/**
 * @brief
 * 채널에 있는 유저들의 파일 디스크립터 목록을 반환하는 함수
 * @return std::vector<int> 유저 파일 디스크립터 목록
 */
std::vector<int>	Channel::GetUserFdList(void)
{
	return (mUserFdList);
}

/**
 * @brief
 * 채널에 연결된 Bot 객체를 반환하는 함수
 * @return Bot* Bot 객체의 포인터
 */
Bot*	Channel::GetBot()
{
	return (mBot);
}

/**
 * @brief
 * 채널의 운영자 파일 디스크립터 목록을 반환하는 함수
 * @return std::vector<int> 운영자 파일 디스크립터 목록
 */
std::vector<int>	Channel::GetOperatorFdList(void)
{
	return (mOperatorFdList);
}

/**
 * @brief
 * 채널의 현재 모드를 문자열로 반환하는 함수
 * @return std::string 채널의 모드 문자열
 */
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

/**
 * @brief
 * 채널의 최대 사용자 제한을 반환하는 함수
 * @return unsigned int 최대 사용자 수
 */
unsigned int	Channel::GetLimit(void)
{
	return (mLimit);
}

/**
 * @brief
 * 채널의 주제를 반환하는 함수
 * @return std::string 채널 주제
 */
std::string	Channel::GetTopic(void)
{
	return (mTopic);
}

/* Setter */
/**
 * @brief
 * 채널의 이름을 설정하는 함수
 * @param channelName 설정할 채널 이름
 */
void	Channel::SetChannelName(std::string channelName)
{
	mChannelName = channelName;
}

/**
 * @brief
 * 채널의 모드를 설정하거나 해제하는 함수
 * @param mode 설정하거나 해제할 모드
 * @param sign '+'로 설정, '-'로 해제
 */
void	Channel::SetMode(unsigned char mode, char sign)
{
	if (sign == '+')
		mMode |= mode;
	else if (sign == '-')
		mMode &= ~mode;
}

/**
 * @brief
 * 채널의 최대 사용자 제한을 설정하는 함수
 * @param num 설정할 사용자 제한 (1부터 시작)
 */
void	Channel::SetLimit(unsigned int num)
{
	mLimit = num + 1;
}

/**
 * @brief
 * 채널의 주제를 설정하는 함수
 * @param topic 설정할 주제
 */
void	Channel::SetTopic(std::string topic)
{
	mTopic = topic;
}

/**
 * @brief
 * 채널의 키를 설정하는 함수
 * @param key 설정할 키
 */
void	Channel::SetKey(std::string key)
{
	mKey = key;
}

/* Check */
/**
 * @brief
 * 채널의 특정 모드가 활성화되었는지 확인하는 함수
 * @param mode 확인할 모드
 * @return true 모드가 활성화된 경우
 * @return false 모드가 비활성화된 경우
 */
bool	Channel::CheckMode(unsigned char mode)
{
	if (mMode & mode)
		return (true);
	return (false);
}

/**
 * @brief
 * 특정 파일 디스크립터가 초대 리스트에 있는지 확인하는 함수
 * @param fd 확인할 파일 디스크립터
 * @return true 초대 리스트에 있는 경우
 * @return false 초대 리스트에 없는 경우
 */
bool	Channel::CheckInvite(int fd)
{
	if (std::find(mInviteFdList.begin(), mInviteFdList.end(), fd) != mInviteFdList.end())
		return (true);
	return (false);
}

/**
 * @brief
 * 주어진 키가 채널의 키와 일치하는지 확인하는 함수
 * @param key 확인할 키
 * @return true 키가 일치하는 경우
 * @return false 키가 일치하지 않는 경우
 */
bool	Channel::CheckKey(std::string key)
{
	if (mKey == key)
		return (true);
	return (false);
}

/**
 * @brief
 * 특정 유저가 채널에 있는지 확인하는 함수
 * @param fd 확인할 유저의 파일 디스크립터
 * @return true 유저가 채널에 있는 경우
 * @return false 유저가 채널에 없는 경우
 */
bool	Channel::CheckUserInChannel(int fd)
{
	std::vector<int>::iterator it = FindMyUserIt(fd);
	if (it != mUserFdList.end())
		return (true);
	return (false);
}

/**
 * @brief
 * 특정 유저가 채널 운영자인지 확인하는 함수
 * @param fd 확인할 유저의 파일 디스크립터
 * @return true 유저가 운영자인 경우
 * @return false 유저가 운영자가 아닌 경우
 */
bool	Channel::CheckOperator(int fd)
{
	if (std::find(mOperatorFdList.begin(), mOperatorFdList.end(), fd) != mOperatorFdList.end())
		return (true);
	return (false);
}

/* Others */
/**
 * @brief
 * 유저 파일 디스크립터를 채널에 추가하는 함수
 * @param fd 추가할 유저의 파일 디스크립터
 */
void	Channel::AppendUserFdList(int fd)
{
	mUserFdList.push_back(fd);
}

/**
 * @brief
 * 유저 파일 디스크립터를 채널에서 제거하는 함수
 * @param fd 제거할 유저의 파일 디스크립터
 */
void	Channel::RemoveUserFdList(int fd)
{
	std::vector<int>::iterator it = FindMyUserIt(fd);
	if (it != mUserFdList.end())
		mUserFdList.erase(it);
}

/**
 * @brief
 * 유저 파일 디스크립터의 위치를 찾는 함수
 * @param fd 찾을 유저의 파일 디스크립터
 * @return std::vector<int>::iterator 유저의 위치 또는 끝 위치
 */
std::vector<int>::iterator	Channel::FindMyUserIt(int fd)
{
	return (std::find(mUserFdList.begin(), mUserFdList.end(), fd));
}

/**
 * @brief
 * 운영자 파일 디스크립터를 추가하는 함수
 * @param fd 추가할 운영자의 파일 디스크립터
 */
void	Channel::AddOperatorFd(int fd)
{
	if (std::find(mOperatorFdList.begin(), mOperatorFdList.end(), fd) == mOperatorFdList.end())
		mOperatorFdList.push_back(fd);
}

/**
 * @brief
 * 운영자 파일 디스크립터를 제거하는 함수
 * @param fd 제거할 운영자의 파일 디스크립터
 */
void	Channel::RemoveOperatorFd(int fd)
{
	std::vector<int>::iterator it = std::find(mOperatorFdList.begin(), mOperatorFdList.end(), fd);
	if (it != mOperatorFdList.end())
		mOperatorFdList.erase(it);
}

/**
 * @brief
 * 초대 리스트에 파일 디스크립터를 추가하는 함수
 * @param fd 추가할 파일 디스크립터
 */
void	Channel::AppendInviteFdList(int fd)
{
	if (std::find(mInviteFdList.begin(), mInviteFdList.end(), fd) == mInviteFdList.end())
		mInviteFdList.push_back(fd);
}
