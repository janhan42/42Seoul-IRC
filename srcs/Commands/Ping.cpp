#include "../Command.hpp"
#include "../Server.hpp"
#include "../User.hpp"

/*
	- RESPONSE LIST -
	ERR_NEEDMOREPARAMS (461)
	ERR_NOORIGIN (409)
 */

// 핑 메세지가 오면 pong 메세지를 돌려주는 함수
// 여기서는 클라이언트가 정기적으로 보내는 핑만 돌려줌
// 유저가 임의로 /ping 명령어를 보내는 경우는 privmsg 에서 처리
void Command::Ping(int fd, std::vector<std::string> commandVec)
{
	/* PING <token> */
	// std::map<int, class User*>& userList = mServer.GetUserList();
	// std::map<int, class User*>::iterator it = userList.find(fd);

	class User* user = mServer.FindUser(fd);

	if (commandVec.size() < 2)
	{
		mResponse.ErrorNeedMoreParams461(*user, commandVec[1]);
		user->AppendUserSendBuf("/PING <token>\r\n");
		return ;
	}
	user->AppendUserSendBuf("PONG " + commandVec[1] + " :" + commandVec[1] + "\r\n");
}
