#ifndef USER_HPP
# define USER_HPP

# include "Server.hpp"
#include <string>

class User
{
	public:
		/* OCCF */
		User(int fd);
		~User();

		// Getter
		std::string							GetNickName(void);
		std::string							GetUserName(void);
		std::string							GetHostName(void);
		std::string							GetServerName(void);
		std::string							GetRealName(void);
		bool								GetIsRegist(void);
		bool								GetPassRegist(void);
		bool								GetNickRegist(void);
		bool								GetUserRegist(void);
		int									GetUserFd(void);
		std::string							GetUserSendBuf(void);
		std::vector<std::string>&			GetChannelList(void);

		// Setter
		void								SetNickName(std::string name);
		void								SetUser(std::string userName, std::string hostName, std::string serverName, std::string realName);
		void								SetPassRegist(bool state);
		void								SetNickRegist(bool state);
		void								SetUserRegist(bool state);
		void								SetRegist(bool state);

		// Append
		void								AppendUserSendBuf(std::string userRecvBuffer);
		void								AppendChannelList(std::string channelName);

		// clear
		void								ClearUserSendBuf(void);
		void								ClearUserSendBuf(int len);
		void								ClearChannelList(void);

		// others
		void								MakeUserToBot(void);
		void								RemoveChannel(std::string channelName);
		std::vector<std::string>::iterator	FindChannel(std::string channelName);
		bool								IsInChannel(const std::string channelName);


	private:
		// User Infomations
		std::string							mNickName;
		std::string							mUserName;
		std::string							mHostName;
		std::string							mServerName;
		std::string							mRealName;

		// regits flags
		bool								mbIsRegistPass;
		bool								mbIsRegistNick;
		bool								mbIsRegistUser;

		// others
		std::string							mUserSendBuf;
		std::vector<std::string>			mChannelList;
		int									mUserFd;


	private: // delete OCCF
		User(const User& rhs);
		User& operator=(const User& rhs);
};
#endif
