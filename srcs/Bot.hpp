#ifndef BOT_HPP
# define BOT_HPP

#include <map>
#include <string>

class Bot
{
	public:
		Bot();
		~Bot();

		/* functions */
		std::string								Introduce(void);;
		std::string								GetHelpBuckShot(void);
		std::string								GetHelpShop(void);
		std::string								GetHelpBuy(void);

		// BuckShot
		class User*								GetFirstUser(void);
		class User*								GetSecondUser(void);
		void									SetFirstUser(class User* firstUser);
		void									SetSecondUser(class User* secondUser);

		// BuckShot
		// std::string BuckShot(class User*& firstUser, class User*& secondUser);

	private:
		std::map<std::string ,std::string>		mCommandList;
		// BuckShot
		class User*							mFirstUser;
		int									mFirstUserHp;
		class User*							mSecondUser;
		int									mSecondUserHp;

	private: // detele OCCF
		Bot(const Bot& rhs);
		Bot& operator=(const Bot& rhs);
};

#endif
