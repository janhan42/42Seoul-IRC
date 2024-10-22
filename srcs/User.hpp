#include <set>
#include <string>

class Channel;

class User
{
  public:  // 생성자, 소멸자
	User(int client_fd);
	~User();

  public:  // member func
	void set_nickname(std::string &name);
	void join_channel(Channel *);

  public:  // member var
	int			fd;
	std::string nickname;

  private:	// unused OCCF
	User();
	User(User &);
	User &operator=(User &);

  private:	// member var
	std::set<Channel *> joined_channel;
};
