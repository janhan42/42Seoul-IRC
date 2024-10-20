#include <netinet/in.h>
#include <sys/event.h>
#include <map>
#include <string>
#include <vector>

#define BUFFER_MAX 1024

class User;

class Server
{
	public:	// 생성자, 소멸자
		Server(int port, const std::string& password);
		~Server();

	public:	// 멤버 함수
		int		init();
		int		run();
		void	handle_message(int client_fd, std::string buffer);

		static	std::vector<std::string> split_message(std::string buffer);
	private: // init Function
		bool	SetServerSock(void);
		bool	SetServerAddr(void);
		bool	SetServerBind(void);

	private: // private member fuction
		int		return_cerr(const std::string &error_message);
		int		init_kq();
		int		accept_new_client();
		void	delete_client(int client_fd);
		void	send_welcome_message(User &new_user);
		void	read_client_message(int client_fd);
		void	send_client_message(int client_fd);

		void	handle_nick(int client_fd, std::string &command);
		bool	is_nickname_taken(const std::string &name);
		bool	is_nickname_invalid(const std::string &name);
		void	send_pass_prompt(int client_fd);

	private: // 멤버 변수
		const int						port;
		const std::string				passwd;
		const std::string				server_name;
		int								serverSock;
		struct sockaddr_in				server_addr;
		int								kq;
		std::map<int, User *>			user_list_by_fd;	// 둘다 서버 종료될 때 delete 필요
		std::map<std::string, User *>	user_list_by_nick;
		std::string						buffer_tmp;
		std::map<int, std::string>		send_buffer;

	private:	// delete OCCF
		Server();
		Server(Server &);
		Server &operator=(Server &);
};
