#ifndef USER_HPP
# define USER_HPP

#include "../../common.hpp"

class User
{
	public:
	/* OCCF */
		User(int);
		~User();
		/* Getter */
		std::string					GetNickName(void);
		std::string 				GetUserName(void);
		std::string					GetHostName(void);
		std::string					GetServerName(void);
		std::string					GetRealName(void);
		bool						GetIsRegist(void);
		bool						GetRegistPass(void);
		bool						GetRegistNick(void);
		bool						GetRegistUser(void);
		int							GetUserFd(void);
		std::string					GetUserRecvBuf(void);
		std::vector<std::string>&	GetChannels(void);
		/* Setter */
		void						SetUser(std::string username, std::string hostname, std::string servername, std::string realname);
		void						SetUserName(const std::string& name);
		void						SetNickName(const std::string& name);
		void						SetHostName(const std::string& name);
		void						SetServerName(const std::string& name);
		void						SetRealNmae(const std::string& name);
		void						SetRegistPass(bool	stste);
		void						SetRegistNick(bool	state);
		void						SetRegistUser(bool state);
		void						SetRegistAll(bool state);
		/* Append */
		void						AppendUserRecvBuf(std::string);
		void						AppendChannels(std::string);
		/* Clear */
		void						ClearUserRecvBuf(void);
		void						ClearChannels(void);
		void						ClearUser(void);
		/* Other */
		void						MakeUserToBot(void);
		void						RemoveChannel(std::string);
		std::vector<std::string>::iterator	FindChannel(std::string);

	private: // variables
		std::string					mNickName;
		std::string					mUserName;
		std::string					mHostName;
		std::string					mServerName;
		std::string					mRealName;

		bool						mbRegistPass;
		bool						mbRegistNick;
		bool						mbRegistUser;

		std::string					mUserRecvBuf;
		std::vector<std::string>	mChannels;
		int							mUserFd;

	private: // delete OCCF
		User(const User& rhs);
		User& operator=(const User& rhs);
};

#endif
