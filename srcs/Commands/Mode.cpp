#include "../Command.hpp"
#include "../User.hpp"

void Command::Mode(int fd, std::vector<std::string> commandVec)
{
	/* NODE <channel> <+/- i, t, k, l, o> (<mode-neede-value>) */
	class User*& user = mServer.GetUserList().find(fd)->second;
	if (commandVec.size() < 2)
	{
		mErrManager.ErrorNeedMoreParams461(*user);
		return;
	}
	Channel* channel = mServer.FindChannel(commandVec[1]);
	if (user->GetNickName() != commandVec[1] && channel == NULL) // channel not exists
	{
		mErrManager.ErrorNosuchChannel403(*user, commandVec[1]);
		return;
	}
	if (channel != NULL && commandVec.size() == 2) // show channel-mode
	{
		user->AppendUserRecvBuf("324 " + user->GetNickName() + " " + commandVec[1] + " +" + channel->GetMode() + "\r\n");
		return;
	}
	if (user->GetNickName() == commandVec[1] && commandVec[2] == "+i")
	{
		user->AppendUserRecvBuf("324 " + user->GetNickName() +  " " + commandVec[1] + " +i" + "\r\n");
		return;
	}
	if (channel != NULL && !channel->CheckOperator(fd)) // if not channel-operator
	{
		mErrManager.ErrorChanOprivsNeeded482(*user, commandVec[1]);
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
	{
		return;
	}
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
				user->AppendUserRecvBuf("324 " + user->GetNickName() + " " + commandVec[1] + " +i" + "\r\n");
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
			std::map<int, class User*>::iterator target = mServer.FindUser(commandVec[modeArgIndex]);
			if (target == mServer.GetUserList().end())	// users not exists
			{
				mErrManager.ErrorNosuchNick401(*user, commandVec[modeArgIndex]);
				return;
			}
			else	// users exists
			{
				if (user->GetNickName() == target->second->GetNickName())
				{
					return;
				}
				if (!channel->CheckUserInChannel(target->second->GetUserFd()))	// users not exist in channel
				{
					mErrManager.ErrorUserNotInChannel441(*user, commandVec[modeArgIndex], commandVec[1]);
					return;
				}
				else if (sign == '+') // give operator-authority
				{
					channel->AddOperatorFd(target->second->GetUserFd());
					isSetMode = true;
					modeArgList.push_back(commandVec[modeArgIndex]);
					modeArgIndex++;
				}
				else if (sign == '-') // remove operator-authority
				{
					channel->RemoveOperatorFd(target->second->GetUserFd());
					isSetMode = true;
					modeArgList.push_back(commandVec[modeArgIndex]);
					modeArgIndex++;
				}
			}
		}
		else	// unknown mode: error
		{
			mErrManager.ErrorUnknownMode472(*user, mode[i]);
			continue;
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
