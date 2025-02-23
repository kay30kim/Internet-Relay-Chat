#ifndef CLIENT_HPP
# define CLIENT_HPP

#include "Irc.hpp"

class Client
{
	private:
		pollfd	_clientPollFd;
	public:
		Client();
		Client(pollfd clientPollFd);
		~Client();
		pollfd getPollFd() const { return _clientPollFd; }
};

#endif