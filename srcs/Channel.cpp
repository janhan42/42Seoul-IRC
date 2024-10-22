#include "Channel.hpp"
#include "User.hpp"

Channel::Channel(const std::string& channel_name) :
	name(channel_name),
	topic("default topic"),
	invite_only(false),
	topic_restrict(false),
	password_set(false),
	limited_user(false)
{
}

Channel::~Channel()
{
	// sdf
}

void Channel::add_user(User* user)
{
	users[user->fd] = user;
	// sdf
}

void Channel::remove_user(User* user)
{
	users.erase(user->fd);
	// sdf
}
