#ifndef CLIENT_HPP
# define CLIENT_HPP

#include "Irc.hpp"

class Client
{
	private:
		pollfd	_client_poll_fd;
	public:
		Client();
		Client(pollfd client_poll_fd);
		~Client();
		pollfd getPollFd() const { return _client_poll_fd; }
};

#endif