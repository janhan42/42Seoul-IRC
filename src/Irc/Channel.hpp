#ifndef CHANNEL_HPP
# define CHANNEL_HPP

/**
 * @file Channel.hpp
 * @author jang hun han (janhan@student.42Seoul.kr)
 * @brief Channel의 관한 선언 및 매크로
 * @version 0.1
 * @date 2024-10-13
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "../../common.hpp"
#include "Bot.hpp"


# define INVITE 1
# define TOPIC	2
# define KEY	4
# define LIMIT	8

class Channel
{
	public:
		/* OCCF */
		Channel(const std::string&, int);
		~Channel();

		/* Getter */
		std::string					GetChannelName(void);
		std::vector<int>			GetUserFdList(void);
		Bot*						GetBot();
		std::vector<int>			GetOperatorFdList(void);
		std::string					GetMode(void);
		unsigned int				GetLimit(void);
		std::string					GetTopic(void);

		/* Setter */
		void						SetChannelName(const std::string& channelName);
		void						SetMode(unsigned char, char);
		void						SetLimit(unsigned int);
		void						SetTopic(const std::string& topic);
		void						SetKey(const std::string& key);
		/* Check */
		bool						CheckMode(unsigned char mode);
		bool						CheckInvite(int fd);
		bool						CheckKey(const std::string& key);
		bool						CheckUserInChannel(int);
		bool						CheckOperator(int);
		/* Other */
		void						AppendUserFdList(int);
		void						RemoveUserFdList(int);
		std::vector<int>::iterator	FindMyUser(int);
		void						AddOperatorFd(int);
		void						RemoveOperatorFd(int);
		void						AppendInviteFdList(int);
	private: // member variables
		// 채널이 가지고 있을 것 뭘까
		Bot*						mBot;
		std::vector<int>			mOperatorFdList;
		std::string					mChannelName;
		std::vector<int>			mUserFdList;
		std::vector<int>			mInviteFdList;
		std::string					mTopic;
		std::string					mKey;
		unsigned char				mMode;
		unsigned int				mLimit;

	private: // delete OCCF
		Channel();
		Channel(const Channel& rhs);
		Channel& operator=(const Channel& rhs);
};

#endif
