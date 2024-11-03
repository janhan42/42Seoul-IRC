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
		const std::string						Introduce(void);;
		const std::string						GetHelpBuckShot(void);
		const std::string						GetHelpAccept(void);
		const std::string						GetHelpReject(void);
		const std::string						GetHelpGame(void);

		// BuckShot
		class User*								GetFirstUser(void);
		class User*								GetSecondUser(void);
		const std::string						GetFirstHpInfo(void);
		const std::string						GetSecondHpInfo(void);
		int										GetFirstHp(void);
		int										GetSecondHp(void);
		void									SetFirstUser(class User* firstUser);
		void									SetSecondUser(class User* secondUser);
		void									SettingGame(void);
		const std::string						SettingChamber(void);
		void									SetReady(bool state);
		bool									GetReady(void);
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
		bool									mbGameReady;
		bool									mbWhoShot;
		std::stack<bool>						mbAmmoChamber;

	private: // detele OCCF
		Bot(const Bot& rhs);
		Bot& operator=(const Bot& rhs);
};

#endif
