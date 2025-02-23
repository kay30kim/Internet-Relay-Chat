#include "Client.hpp"

Client::Client()
{
	std::cout <<"Client constructor\n";
	memset(&_client_poll_fd, 0, sizeof(_client_poll_fd));
}

Client::Client(pollfd client_poll_fd) : _client_poll_fd(client_poll_fd)
{
	std::cout << "Client constructor with clientPollFd\n";
}

Client::~Client()
{
	std::cout << "Client destructor\n";
}
