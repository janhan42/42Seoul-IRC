#include "Bot.hpp"
#include "Command.hpp"

Bot::Bot()
{}

Bot::~Bot()
{}

std::string Bot::Introduce()
{
	return "@BOT + 명령어로 명령 실행이 가능합니다. \t # 사용가능 명령어 리스트 보기 : list# #\t 명령어 추가하기 : add + 명령어 이름 + 명령어 내용# \t # 명령어 실해이기 : do + 명령어 이름 #";
}

void Bot::AddComamnd(std::string command, std::string response)
{
	mCommandList[command] = response;
}

void Bot::DelCommand(std::string command)
{
	mCommandList.erase(command);
}

std::string Bot::Docommand(std::string command)
{
	std::map<std::string, std::string>::iterator it = mCommandList.find(command);
	if (it != mCommandList.end())
		return it->second;
	return "없는 명령어 입니다.";
}

std::vector<std::string> Bot::GetCommandList()
{
	std::vector<std::string> list;
	std::map<std::string, std::string>::iterator it = mCommandList.begin();
	for(; it != mCommandList.end(); it++)
		list.push_back(it->first);
	return list;
}
