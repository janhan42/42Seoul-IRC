#ifndef ERRDEFINE_HPP
# define ERRDEFINE_HPP

# define MAX_USER 100
# define SERVERNAME "SIRC"
# define BOT -1
# define MAX_EVENT 10

/* RPL */
# define RPL_ENDOFNAMES ":End of /NAMES list\r\n"

/* PASS */
# define ERR_NEEDMOREPARAMS "Not enough parameters\r\n"
# define ERR_ALREADYREGISTERED "You may not reregister\r\n"
# define ERR_PASSWDMISMATCH "Password incorrect\r\n"

/* NICK */
# define ERR_NONICKNAMEGIVEN "No nickname given\r\n"
# define ERR_ERRONEUSNICKNAME "Erroneus nickname\r\n"
# define ERR_NICKNAMEINUSE "Nickname is already in use\r\n"
# define ERR_NICKCOLLISION "Nickname collision KILL from\r\n"

/* PRIVMSG */
# define ERR_NOSUCHNICK "No such nick/channel\r\n"
# define ERR_NOSUCHSERVER "No such server\r\n"
# define ERR_CANNOTSENDTOCHAN "Cannot send to channel\r\n"
# define ERR_NORECIPIENT "No recipient given\r\n"
# define ERR_NOTEXTTOSEND "No text to send\r\n"

// PART
# define ERR_NOSUCHCHANNEL "No such channel\r\n"
# define ERR_NOTONCHANNEL "You're not on that channel\r\n"

// KICK
# define ERR_CHANOPRIVSNEEDED "You're not channel operator\r\n"
# define ERR_USERNOTINCHANNEL "They aren't on that channel\r\n"

// COMMON
# define ERR_NOTREGISTERED "You have not registered\r\n"

// MODE
# define ERR_UMODEUNKNOWNFLAG "Unknown MODE flag\r\n"
# define ERR_USERSDONTMATCH "Can't change mode for other users\r\n"
# define ERR_UNKNOWNMODE "is unknown mode char to me\r\n"
# define ERR_CHANNELISFULL "Cannot join channel (+l)\r\n"

// JOIN
# define ERR_INVITEONLYCHAN "Cannot join channel (+i)\r\n"
# define ERR_BADCHANNELKEY "Cannot join channel (+k)\r\n"

// TOPIC
# define RPL_NOTOPIC "No topic is set\r\n"

// INVITE
# define ERR_USERONCHANNEL "is already on channel\r\n"
# define RPL_INVITING "Inviting\r\n"

#endif
