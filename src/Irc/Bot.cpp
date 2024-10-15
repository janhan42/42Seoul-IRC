#include "Bot.hpp"

Bot::Bot()
{}

Bot::~Bot()
{}

std::string	Bot::introduce()
{
	return "안녕하세요! \t @BOT + 명령어로 명령실행이 가능합니다. \t # 사용가능한 명령어 리스트보기 : list#  # \t 명령어 추가하기 : add + 명령어 이름 + 명령 내용# \t # 명령어 삭제하기 : del + 명령어 이름# \t # 명령어 실행하기 : do + 명령어 이름# ";
}

void Bot::addCommand(const std::string& command, const std::string& response)
{
	mCommands[command] = response;
}

void Bot::deleteCommand(const std::string& command)
{
	mCommands.erase(command);
}

std::string Bot::doCommand(const std::string& command)
{
	std::map<std::string, std::string>::iterator it = mCommands.find(command);
	if (it != mCommands.end())
		return it->second;
	return "명령어를 찾을 수 없습니다.";
}

std::vector<std::string> Bot::listCommand()
{
	std::vector<std::string> list;
	for (std::map<std::string, std::string>::iterator it = mCommands.begin(); it != mCommands.end(); ++it)
		list.push_back(it->first);
	return list;
}
