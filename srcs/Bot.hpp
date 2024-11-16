#ifndef BOT_HPP
# define BOT_HPP

#include <map>
#include <string>
#include <stack>

class Server;
class Command;
class Channel;

class Bot
{
	public:
		Bot();
		~Bot();

		/* functions */
		const std::string						Introduce(void);
		void									HelpMsgtoChannel(Command* command, std::string channel);

		// BuckShot
		void									ValidTarget(int fd, Server* server, Channel* channel, Command* command, std::vector<std::string> commandVec);
		void									ResponseGameRequest(Channel* channel, Command* command, std::vector<std::string> commandVec);
		void									GameControl(int fd, Channel* channel, Command* command, std::vector<std::string> commandVec);
		void									HandleGameTurn(int fd, Command* command, Channel* channel,  std::vector<std::string> commandVec);
		void									GameInfo(Command* command, Channel* channel);
		void									CheckWinner(Command* command, Channel* channel);
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
