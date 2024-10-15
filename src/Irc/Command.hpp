#ifndef COMMAND_HPP
# define COMMAND_HPP

#include <map>
#include <sstream>
#include "./User.hpp"
#include "../server/Server.hpp"

class Channel;

class Command
{
public:
	/* OCCF */
	Command(Server &server);
	~Command();

	/* member functions */
	// verify commands
	void		Excute(int fd);
	// commands/
	void		Pass(int fd, std::vector<std::string> commandVector);
	void		Nick(int fd, std::vector<std::string> commandVector);
	void		User(int fd, std::vector<std::string> commandVector);
	void		Ping(int fd, std::vector<std::string> commandVector);
	void		Privmsg(int fd, std::vector<std::string> commandVector);
	void		Quit(int fd, std::vector<std::string> commandVector);
	void		Part(int fd, std::vector<std::string> commandVector);
	void		Join(int fd, std::vector<std::string> commandVector);
	void		Kick(int fd, std::vector<std::string> commandVector);
	void		Mode(int fd, std::vector<std::string> commandVector);
	void		Topic(int fd, std::vector<std::string> commandVector);
	void		Invite(int fd, std::vector<std::string> commandVector);
	// utils in commands/
	void		BotCommand(int fd, std::vector<std::string> commandVector);
	void		TopicMsg(int fd, std::string message);
	bool		CheckNicknameDuplicate(std::string message, std::map<int, User>& userList);
	bool		CheckNicknameValidate(std::string nickName);
	bool		CheckRealname(std::string realName);
	bool		CheckBotCommand(std::string command);
	// utils in Util.cpp
	std::string	ChannelMessage(int fd, std::vector<std::string> commandVector);
	void		ChannelPRIVMSG(std::string massege, User& user, Channel* channel);
	void		ChannelPART(int fd, std::string channelName, std::vector<std::string> commandVector);
	void		MsgToAllChannel(int target, std::string channelName, std::string command, std::string massege);
	std::string	MakeFullName(int fd);
	void		NameListMsg(int fd, std::string channelName);

private:
	/* OCCF */
	Command &operator=(const Command&);
	Command(const Command&);

	/* member variables */
	Server		&mServer;
};
#endif
