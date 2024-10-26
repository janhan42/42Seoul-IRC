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
		std::string								Introduce(void);
		void									AddComamnd(std::string, std::string);
		void									DelCommand(std::string);
		std::vector<std::string>				GetCommandList(void);
		std::string								Docommand(std::string);

	private:
		std::map<std::string ,std::string>		mCommandList;

	private: // detele OCCF
		Bot(const Bot& rhs);
		Bot& operator=(const Bot& rhs);
};

#endif
