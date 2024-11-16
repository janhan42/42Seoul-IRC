#include "../Command.hpp"
#include "../User.hpp"
#include <iostream>

void Command::Mode(int fd, std::vector<std::string> commandVec)
{
	/* NODE <channel> <+/- i, t, k, l, o> (<mode-neede-value>) */
	//class User*& user = mServer.GetUserList().find(fd)->second;
	class User* user = mServer.FindUser(fd);
	if (commandVec.size() < 2)
	{
		mResponse.ErrorNeedMoreParams461(*user, commandVec[1]);
		return;
	}
	Channel* channel = mServer.FindChannel(commandVec[1]);
	if (user->GetNickName() != commandVec[1] && channel == NULL) // channel not exists
	{
		mResponse.ErrorNosuchChannel403(*user, commandVec[1]);
		return;
	}
	if (channel != NULL && commandVec.size() == 2) // show channel-mode
	{
		user->AppendUserSendBuf("324 " + user->GetNickName() + " " + commandVec[1] + " +" + channel->GetMode() + "\r\n");
		return;
	}
	if (user->GetNickName() == commandVec[1] && commandVec[2] == "+i")
	{
		user->AppendUserSendBuf("324 " + user->GetNickName() +  " " + commandVec[1] + " +i" + "\r\n");
		return;
	}
	if (commandVec[2] == "b")
	{
		return;
	}
	if (channel != NULL && !channel->CheckOperator(fd)) // if not channel-operator
	{
		mResponse.ErrorChanOprivsNeeded482(*user, commandVec[1]);
		return;
	}
	std::string mode = commandVec[2];
	char sign;
	int index = 0;
	while (mode[index] == '+' || mode[index] == '-')
	{
		sign = mode[index];
		index++;
	}
	if (mode.length() == 1)
		return;
	std::string msg = "";
	std::vector<std::string> modeArgList;
	unsigned int modeArgIndex = 3;
	for (size_t i = index; i < mode.length(); i++)
	{
		if (mode[i] == '+' || mode[i] == '-')
			break;
		bool isSetMode = false;
		if (mode[i] == 'i')		// invite
		{
			if (sign == '+' && channel->CheckMode(INVITE))
				continue;
			if (sign == '-' && !channel->CheckMode(INVITE))
				continue;
			if (user->GetNickName() == commandVec[1])
			{
				user->AppendUserSendBuf("324 " + user->GetNickName() + " " + commandVec[1] + " +i" + "\r\n");
				return;
			}
			channel->SetMode(INVITE, sign);
			isSetMode = true;
		}
		else if (mode[i] == 't')	// topic
		{
			if (sign == '+' && channel->CheckMode(TOPIC))
				continue;
			if (sign == '-' && !channel->CheckMode(TOPIC))
				continue;
			channel->SetMode(TOPIC, sign);
			isSetMode = true;
		}
		else if (mode[i] == 'k')	// key
		{
			if (sign == '-' && !channel->CheckMode(KEY))
				continue;
			if (commandVec.size() > modeArgIndex)
			{
				channel->SetMode(KEY, sign);
				if (sign == '+')
				{
					channel->SetKey(commandVec[3]);
				}
				else if (sign == '-')
				{
					channel->SetKey("");
				}
				isSetMode = true;
				modeArgList.push_back(commandVec[modeArgIndex]);
				modeArgIndex++;
			}
		}
		else if (mode[i] == 'l')	// limit
		{
			if (sign == '-' && !channel->CheckMode(LIMIT))
				continue;
			else if (sign == '-' && channel->CheckMode(LIMIT))
			{
				channel->SetMode(LIMIT, sign);
				isSetMode = true;
			}
			if (commandVec.size() > modeArgIndex)
			{
				std::string limit_s = commandVec[modeArgIndex].c_str();
				bool isDigit = true;
				for (size_t j = 0; j < limit_s.length(); ++j)
				{
					if (!isdigit(limit_s[j]))
					{
						isDigit = false;
						break;
					}
				}
				if (!isDigit)
				{
					modeArgIndex++;
					continue;
				}
				int limit = atoi(limit_s.c_str());
				if (limit < 0)
				{
					modeArgIndex++;
					continue;
				}
				channel->SetMode(LIMIT, sign);
				if (sign == '+')
				{
					channel->SetLimit(limit);
				}
				isSetMode = true;
				modeArgList.push_back(commandVec[modeArgIndex]);
				modeArgIndex++;
			}
		}
		else if (mode[i] == 'o')	// operator
		{
			if (commandVec.size() <= modeArgIndex)
			{
				continue;
			}
			class User* target = mServer.FindUser(commandVec[modeArgIndex]);
			if (target == NULL)	// users not exists
			{
				mResponse.ErrorNosuchNick401(*user, commandVec[modeArgIndex]);
				return;
			}
			else	// users exists
			{
				if (user->GetNickName() == target->GetNickName())
				{
					return;
				}
				if (!channel->CheckUserInChannel(target->GetUserFd()))	// users not exist in channel
				{
					mResponse.ErrorUserNotInChannel441(*user, commandVec[modeArgIndex], commandVec[1]);
					return;
				}
				else if (sign == '+') // give operator-authority
				{
					channel->AddOperatorFd(target->GetUserFd());
					isSetMode = true;
					modeArgList.push_back(commandVec[modeArgIndex]);
					modeArgIndex++;
				}
				else if (sign == '-') // remove operator-authority
				{
					channel->RemoveOperatorFd(target->GetUserFd());
					isSetMode = true;
					modeArgList.push_back(commandVec[modeArgIndex]);
					modeArgIndex++;
				}
			}
		}
		if (isSetMode)
		{
			if (msg.empty())
				msg += sign;
			msg += mode[i];
		}
	}
	for (size_t i = 0; i < modeArgList.size(); ++i)
	{
		msg += " " + modeArgList[i];
	}
	if (msg.empty())
		return;
	MsgToAllChannel(fd, commandVec[1], "MODE", msg);
}
