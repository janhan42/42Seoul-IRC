#ifndef ResponseManager_HPP
# define ResponseManager_HPP

#include <string>
#include "../Define.hpp"

class User;

class ResponseManager
{
	public:
		/* OCCF */
		ResponseManager();
		~ResponseManager();

		void RPLWelcome001(User& user, std::string FUllName);
		void RPLYourHost002(User& user);
		void RPLCreated003(User& user);
		void RPLMyInfo004(User& user);
		void RPLISupport005(User& user);

		void ErrorNoTopic331(User& user, std::string channel);
		void RPL_NamReply353(User& user, std::string channel, std::string meassage);
		void RPL_EndOfNames366(User& user, std::string channel);

		/* No Such Error */
		void ErrorNosuchNick401(User& user, std::string nickName);
		void ErrorNosuchChannel403(User& user, std::string channel);;

		void ErrorNoTextToSend412(User& user);
		void RPLNoMotd422(User& user);
		void ErrorNoNickNameGiven431(User& user);

		/* Channel Error */
		void ErrorUserNotInChannel441(User& user, std::string nickName, std::string channel);
		void ErrorNotOnChannel442(User& user, std::string channel);
		void ErrorUserOnChannel443(User& user, std::string nickName, std::string channel);


		void ErrorNeedMoreParams461(User& user, std::string command);
		void ErrorAlreadyRegistRed462(User& user);
		void ErrorPasswdMisMatch464(User& user);

		void ErrorNotRegistered451(User& user);
		void ErrorErronusNickName432(User& user, std::string name);
		void ErrorNickNameInuse433(User& user, std::string name);

		/* mode 47* Error */
		void ErrorChannelIsFull471(User& user, std::string channel);
		void ErrorUnknownMode472(User& user, char mode);
		void ErrorInviteOnlychan473(User& user, std::string channel);
		void ErrorBadChannelKey475(User& user, std::string channel);

		/* mode 48* Error */
		void ErrorChanOprivsNeeded482(User& user, std::string channel);

		void ErrorUsersDontMatch502(User& user);


	private: // delete OCCF
		ResponseManager(const ResponseManager& rhs);
		ResponseManager& operator=(const ResponseManager& rhs);

};
#endif
