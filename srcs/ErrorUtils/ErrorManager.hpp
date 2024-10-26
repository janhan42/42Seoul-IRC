#ifndef ERRORMANAGER_HPP
# define ERRORMANAGER_HPP

#include <string>
#include "../ErrDefine.hpp"

class User;

class Errormanager
{
	public:
		/* OCCF */
		Errormanager();
		~Errormanager();

		/* No Such Error */
		void ErrorNosuchNick401(User& user, std::string nickName);
		void ErrorNosuchChannel403(User& user, std::string channel);;

		/* Channel Error */
		void ErrorUserNotInChannel441(User& user, std::string nickName, std::string channel);
		void ErrorNotOnChannel442(User& user, std::string channel);
		void ErrorUserOnChannel443(User& user, std::string nickName, std::string channel);


		void ErrorAlreadyRegistRed462(User& user);
		void ErrorNeedMoreParams461(User& user);
		void ErrorPasswdMisMatch464(User& user);

		/* mode 47* Error */
		void ErrorChannelIsFull471(User& user, std::string channel);
		void ErrorUnknownMode472(User& user, char mode);
		void ErrorInviteOnlychan473(User& user, std::string channel);
		void ErrorBadChannelKey475(User& user, std::string channel);

		/* mode 48* Error */
		void ErrorChanOprivsNeeded482(User& user, std::string channel);

		void ErrorNotRegistered451(User& user);
	private: // delete OCCF
		Errormanager(const Errormanager& rhs);
		Errormanager& operator=(const Errormanager& rhs);

};
#endif
