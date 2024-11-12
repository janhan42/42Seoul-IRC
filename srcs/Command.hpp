#pragma once
#ifndef COMMAND_HPP
# define COMMAND_HPP

#include <map>

#include "Channel.hpp"
#include "ErrorUtils/ErrorManager.hpp"

class User;
class Server;
class Channel;

class Command
{
	public:
		/* OCCF */
		Command(Server& server);
		~Command();

		void Run(int fd);

		/* Commands */
		void		Pass(int fd, std::vector<std::string> commandVec);
		void		Nick(int fd, std::vector<std::string> commandVec);
		void		User(int fd, std::vector<std::string> commandVec);
		void		Ping(int fd, std::vector<std::string> commandVec);
		void		Privmsg(int fd, std::vector<std::string> commandVec);
		void		Quit(int fd, std::vector<std::string> commandVec);
		void		Part(int fd, std::vector<std::string> commandVec);
		void		Join(int fd, std::vector<std::string> commandVec);
		void		Kick(int fd, std::vector<std::string> commandVec);
		void		Mode(int fd, std::vector<std::string> commandVec);
		void		Topic(int fd, std::vector<std::string> commadVec);
		void		Invite(int fd, std::vector<std::string> commanVec);
		void		Notice(int fd, std::vector<std::string> commandVec);

		/* Utils in commands */
		void		BotCommand(int fd, std::vector<std::string> commandVeco);
		void		TopicMsg(int fd, std::string channelName);
		bool		CheckNickNameDuplicate(std::string name, std::map<int, class User*>& userList);
		bool		CheckNickNameValidate(std::string name);
		bool		CheckRealName(std::string name);
		bool		CheckBotCommand(std::string command);

		/* Uitls in Uils.cpp */
		std::string	ChannelMessage(int messageIndex, std::vector<std::string> commandVec);
		void		ChannelPrivmsg(std::string, class User&, Channel*);
		void		ChannelPART(int fd, std::string cannelName, std::vector<std::string> commandVec);
		void		MsgToAllChannel(int target, std::string channelName, std::string command, std::string msg);
		void		NickMsgToAllChannel(int target, std::string channelName, std::string command, std::string msg);
		std::string	MakeFullName(int fd);
		void		NameListMsg(int fd, std::string);

	private: // variables
		void RegistNewUser(int &fd, std::map<int, class User *> &userList, std::map<int, class User *>::iterator &iter, std::vector<std::string> &commandVec);
		Server&			mServer;
		Errormanager	mErrManager;


	private: // delete OCCF
		Command();
		Command(const Command& rhs);
		Command& operator=(const Command& rhs);
};

#endif
