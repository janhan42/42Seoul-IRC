#include <map>
#include <string>

class User;

class Channel
{
  public:  // OCCF
	Channel(const std::string& channel_name);
	~Channel();

	void add_user(User* user);
	void remove_user(User* user);

  private:	// OCCF
	Channel();
	Channel(Channel&);
	Channel& operator=(Channel&);

  public:	// member function
  private:	// member function
  private:	// member attribute
	const std::string	 name;
	std::string			 topic;
	std::map<int, User*> users;
	std::map<int, User*> operators;

	bool invite_only;
	bool topic_restrict;
	bool password_set;
	bool limited_user;
};
