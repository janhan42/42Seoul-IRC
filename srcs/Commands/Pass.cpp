
#include <unistd.h>
#include "../Command.hpp"
#include "../Server.hpp"
#include "../User.hpp"

/*
	- RESPONSE LIST -
	ERR_NEEDMOREPARAMS (461)
	ERR_ALREADYREGISTERED (462)
	ERR_PASSWDMISMATCH (464)
 */

// 유저가 pass 명령어로 전달한 암호와 서버의 암호가 일치하는지 확인하는 함수
// 암호가 틀렸을 경우 유저 서버에서 삭제한다
// pass 뒤에 암호가 오지 않는 경우는 없다고 봐도 되는게,
// 접속시에 암호를 입력하지 않으면 nick 명령어부터 들어오기 때문에
// nick 함수에서 GetPassRegist 체크를 통과못하고 유저 삭제됨
//
// 어쨌든 pass를 통과해야 nick 명령어를 받을 수 있도록 설계됨
void Command::Pass(int fd, std::vector<std::string> commandVec)
{
	/* PASS <password> */
	// std::map<int, class User*>& userList = mServer.GetUserList();
	// std::map<int, class User*>::iterator it = userList.find(fd);

	class User* user = mServer.FindUser(fd);
	std::string password = mServer.GetPassWord();
	if (user->GetPassRegist())
	{
		mResponse.ErrorAlreadyRegistRed462(*user);
		return ;
	}
	if (commandVec.size() < 2)
	{
		mResponse.ErrorNeedMoreParams461(*user, commandVec[1]);
		return ;
	}
	//if (strcmp(commandVec[1].c_str(), password.c_str()) != 0)
	if (commandVec[1] != password)
	{
		mResponse.ErrorPasswdMisMatch464(*user);
		send(fd, user->GetUserSendBuf().c_str(), user->GetUserSendBuf().length(), 0);
		// delete it->second;
		// userList.erase(fd);
		// close(fd);
		mServer.DeleteUserFromServer(fd);
		return ;
	}
	user->SetPassRegist(true);
}
