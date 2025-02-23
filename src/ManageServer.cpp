#include "Server.hpp"

static int	acceptSocket(int listenSocket)
{
	sockaddr_in	client;
	socklen_t	addr_size = sizeof(sockaddr_in);
	return (accept(listenSocket, (sockaddr *)&client, &addr_size));
}

static void	addClient(int client_socket, std::vector<pollfd> poll_fds)
{
	pollfd	client_pollfd;
	client_pollfd.fd = client_socket;
	client_pollfd.events = POLLIN;
	poll_fds.push_back(client_pollfd);
	std::cout << "ADD CLIENT SUCCESSED" << std::endl;
}

static void	tooManyClients(int client_socket)
{
	std::cout << ERR_FULL_SERV << std::endl;
	send(client_socket, ERR_FULL_SERV, strlen(ERR_FULL_SERV) + 1, 0);
	close(client_socket);
}

static sockaddr_in	getClientAddress(int socket)
{
	sockaddr_in client;
	socklen_t addrSize = sizeof(struct sockaddr_in);
	if (getpeername(socket, (struct sockaddr*)&client, &addrSize) != SUCCESS)
	{
		std::cerr << "Get Client Address Error\n";
		throw ;
	}
	return (client);
}

static void	print(std::string type, int client_socket, const char *message)
{
	sockaddr_in client = getClientAddress(client_socket);
	std::cout << type \
			  << client_socket << " " \
			  << inet_ntoa(client.sin_addr) << " " \
			  << ntohs(client.sin_port) \
			  << message;
}

static void	delClient(std::vector<pollfd> poll_fds, std::vector<pollfd>::iterator it)
{
	poll_fds.erase(it);
	print("Deconnection : ", it->fd, static_cast<const char*>("\n"));
	close(it->fd);
	std::cout << "Client deleted \nTotal Client is now: " << (unsigned int)(poll_fds.size() - 1) << std::endl;
}

int		Server::manageServerLoop()
{
	pollfd	server_poll_fd;
	
	server_poll_fd.fd = _server_socket_fd;
	server_poll_fd.events = POLLIN;
	std::vector<pollfd>	poll_fds;
	poll_fds.push_back(server_poll_fd);
	while (1)
	{
		if (poll((pollfd *)&poll_fds[0], (unsigned int)poll_fds.size(), -1) <= SUCCESS) // -1 == no timeout
		{
			std::cerr << "poll error\n";
			return (FAILURE);
		}
		std::vector<pollfd>::iterator	it;
		std::vector<pollfd>::iterator	end = poll_fds.end();
		for (it = poll_fds.begin(); it != end; it++)
		{
			if (it->revents & POLLIN)
			{
				if (it->fd == _server_socket_fd)
				{
					int	client_sock = acceptSocket(_server_socket_fd);
					if (client_sock == -1)
					{
						std::cerr << "Accept failed\n";
						continue;
					}
					if (poll_fds.size() - 1 < MAX_CLIENT_NB)
						addClient(client_sock, poll_fds);
					else
						tooManyClients(_server_socket_fd);
				}
				else
				{
					// client echo message
					char	message[BUF_SIZE_MSG];
					if (recv(it->fd, message, BUF_SIZE_MSG, 0) <= FAILURE)
						delClient(poll_fds, it);
					else
					{
						print("Recv : ", it->fd, message);
						send(it->fd, message, strlen(message) + 1, 0);
						print("Send : ", it->fd, message);
					}
				}
			}
			else if (it->revents & POLLERR)
			{
				if (it->fd == _server_socket_fd)
				{
					std::cerr << "Lister socket error\n";
					return (FAILURE);
				}
				else
					delClient(poll_fds, it);
			}
		}
	}
	return (SUCCESS);
}