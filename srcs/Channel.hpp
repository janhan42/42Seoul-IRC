#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include <string>
#include <vector>

# define INVITE	1
# define TOPIC	2
# define KEY	4
# define LIMIT	8

class Bot;

class Channel
{
	public:
		Channel(const std::string& channelName, int fd);
		~Channel();

		// Getter
		std::string							GetChannelName(void);
		std::vector<int>					GetUserFdList(void);
		Bot*								GetBot(void);
		std::vector<int>					GetOperatorFdList(void);
		std::string							GetMode(void);
		unsigned int						GetLimit(void);
		std::string							GetTopic(void);

		// Setter
		void								SetChannelName(std::string channelName);
		void								SetMode(unsigned char mode, char sign);
		void								SetLimit(unsigned int num);
		void								SetTopic(std::string topic);
		void								SetKey(std::string key);

		// check
		bool								CheckMode(unsigned char mode);
		bool								CheckInvite(int fd);
		bool								CheckKey(std::string key);
		bool								CheckUserInChannel(int fd);
		bool								CheckOperator(int fd);

		// others
		void								AppendUserFdList(int fd);
		void								RemoveUserFdList(int fd);
		std::vector<int>::iterator			FindMyUserIt(int fd);
		void								AddOperatorFd(int fd);
		void								RemoveOperatorFd(int fd);
		void								AppendInviteFdList(int fd);


	private: // variable
		Bot*								mBot;
		std::vector<int>					mOperatorFdList;
		std::string							mChannelName;
		std::vector<int>					mUserFdList;
		std::vector<int>					mInviteFdList;
		std::string							mTopic;
		std::string							mKey;
		unsigned char						mMode;
		unsigned int						mLimit;

	private: // delete OCCF
		Channel();
		Channel(const Channel& rhs);
		Channel& operator=(const Channel& rhs);
};

#endif
