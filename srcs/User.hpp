#include <string>

class User
{
  public:  // 생성자, 소멸자
	User(int client_fd);
	~User();

  public:  // member func
	void set_nickname(std::string &name);

  public:  // member var
	int			fd;
	std::string nickname;

  private:	// unused OCCF
	User();
	User(User &);
	User &operator=(User &);

  private:						  // member var
	std::string current_channel;  // 수정 필요
};
