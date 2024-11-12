#include "../Command.hpp"
#include "../Server.hpp"
#include "../User.hpp"
#include <iostream>
#include <sstream>

void printRawString2(const std::string& str) {
    std::ostringstream oss;

    for (char ch : str) {
        if (isprint(ch) || ch == '\n' || ch == '\r' || ch == '\t') {
            // 출력할 수 있는 문자들, 공백이나 탭 등도 그대로 출력
            oss << ch;
        } else {
            // 제어문자일 경우, \xHH 형식으로 출력
            oss << "\\x" << std::hex << std::uppercase << (0xFF & static_cast<int>(ch));
        }
    }

    std::cout << "Raw String Output: [" << oss.str() << "]" << std::endl;
}

void Command::Notice(int fd, std::vector<std::string> commandVec)
{
	// /NOTICE <target>{,<target>} <text to be sent>
	std::map<int, class User*>& userList = mServer.GetUserList();
	std::map<int, class User*>::iterator userIt = userList.find(fd);
	/* TESTOUTPUT */
	for (std::vector<std::string>::iterator it = commandVec.begin(); it != commandVec.end(); it++)
	{
		std::cout << "NOTICE TEST : [" << *it << "]" << std::endl;
	}
	/* END */
	if (commandVec.size() < 2)
	{
		mErrManager.ErrorNeedMoreParams461(*userIt->second, commandVec[1]);
		return;
	}
	if (commandVec[2].empty())
	{
		mErrManager.ErrorNoTextToSend412(*userIt->second);
		return;
	}
	printRawString2(commandVec[2]);
	if (commandVec[2] == ":\x1PING\0")
	{
		std::map<int, class User*>::iterator target = mServer.FindUser(commandVec[1]);
		if (target == mServer.GetUserList().end())
		{
			mErrManager.ErrorNosuchNick401(*userIt->second, commandVec[1]);
		}

	}
}
