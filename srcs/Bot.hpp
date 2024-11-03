#ifndef BOT_HPP
# define BOT_HPP

#include <map>
#include <string>
#include <stack>

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
		void									SettingGame(void);
		void									ClearGame(void);
		bool									GameOn(void);
		bool									GetWhoShot(void);
		void									SetWhoShot(bool state);
		const std::string						GameShot(const std::string& state);
		int										AmmoCount(void);

	private:
		std::map<std::string ,std::string>		mCommandList;
		// BuckShot
		class User*								mFirstUser;
		int										mFirstUserHp;
		class User*								mSecondUser;
		int										mSecondUserHp;
		bool									mbGameOn;
		bool									mbWhoShot;
		std::stack<bool>						mbAmmoChamber;

	private: // detele OCCF
		Bot(const Bot& rhs);
		Bot& operator=(const Bot& rhs);
};

#endif
