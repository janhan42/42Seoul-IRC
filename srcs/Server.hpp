#include <netinet/in.h>
#include <string>
#include <vector>
#include "User.hpp"

class Server
{
  public:  // 생성자, 소멸자
	Server(int port, std::string passwd);
	~Server();

  public:  // 멤버 함수
	int init();
	int run();

  private:	// private member fuction
	int return_cerr(const std::string &error_message);

	int init_kq();

  private:	// delete OCCF
	Server();
	Server(Server &);
	Server &operator=(Server &);

  private:	// 멤버 변수
	int					server_fd;
	const int			port;
	const std::string	passwd;
	struct sockaddr_in	server_addr;
	const std::string	server_name;
	std::vector<User *> user_list;
};
