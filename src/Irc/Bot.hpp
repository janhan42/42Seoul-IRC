#ifndef BOT_HPP
# define BOT_HPP

#include "../../common.hpp"

class Bot
{
	public:
		Bot();
		~Bot();

		std::string					introduce();
		void						addCommand(const std::string& command, const std::string& response);
		void						deleteCommand(const std::string& command);
		std::vector<std::string>	listCommand();
		std::string					doCommand(const std::string& command);
	private:
		std::map<std::string, std::string> mCommands;

	private: // delete OCCF
		Bot(const Bot& rhs);
		Bot& operator=(const Bot& rhs);
};

#endif
