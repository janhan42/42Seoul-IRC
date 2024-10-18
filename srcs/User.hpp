#include <string>

class User
{
  public:  // 생성자, 소멸자
	User(int client_fd);
	~User();

  public:	// member func
  private:	// unused OCCF
	User();
	User(User &);
	User &operator=(User &);

  private:	// member var
	std::string nickname;
	std::string current_channel;
	int			fd;
};
