#include <netinet/in.h>
#include <sys/event.h>
#include <string>
#include <unordered_map>
#include <vector>

#define BUFFER_MAX 1024

class User;

class Server
{
  public:  // 생성자, 소멸자
	Server(int port, std::string passwd);
	~Server();

  public:  // 멤버 함수
	int	 init();
	int	 run();
	void handle_message(int client_fd, std::string buffer);

	static std::vector<std::string> split_message(std::string buffer);

  private:	// private member fuction
	int	 return_cerr(const std::string &error_message);
	int	 init_kq();
	int	 accept_new_client(int kq, struct kevent *evSet);
	void send_welcome_message(User &new_user);
	void read_client_message(int client_fd);

	void handle_nick(int client_fd, std::string &command);

  private:	// delete OCCF
	Server();
	Server(Server &);
	Server &operator=(Server &);

  private:	// 멤버 변수
	int				   server_fd;
	const int		   port;
	const std::string  passwd;
	struct sockaddr_in server_addr;
	const std::string  server_name;
	// std::vector<User *> user_list;
	std::unordered_map<int, User *> user_list;
	// char				readbuffer[BUFFER_MAX];
	std::string buffer;
};
