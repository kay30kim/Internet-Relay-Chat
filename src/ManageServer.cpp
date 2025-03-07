#include "Server.hpp"
#include "Colors.hpp"

static int acceptSocket(int listenSocket)
{
	sockaddr_in client;
	socklen_t addr_size = sizeof(sockaddr_in);
	return (accept(listenSocket, (sockaddr *)&client, &addr_size));
}

static void	addClient(int client_socket, std::vector<pollfd> &poll_fds)
{
	pollfd	client_pollfd;
	client_pollfd.fd = client_socket;
	client_pollfd.events = POLLIN | POLLOUT;
	poll_fds.push_back(client_pollfd);
	std::cout << PURPLE << "ADDED CLIENT SUCCESSFULLY" << RESET << std::endl;
}

static void	tooManyClients(int client_socket)
{
	std::cout << RED << ERR_FULL_SERV << RESET << std::endl;
	send(client_socket, ERR_FULL_SERV, strlen(ERR_FULL_SERV) + 1, 0);
	close(client_socket);
}

static sockaddr_in getClientAddress(int socket)
{
	sockaddr_in client;
	socklen_t addrSize = sizeof(struct sockaddr_in);
	if (getpeername(socket, (struct sockaddr *)&client, &addrSize) != SUCCESS)
	{
		std::cerr << RED << "Get Client Address Error" << RESET << std::endl;
		throw;
	}
	return (client);
}

static void print(std::string type, int client_socket, char *message)
{
	sockaddr_in client = getClientAddress(client_socket);
	std::cout << PURPLE << type
			  << RESET << client_socket << " "
			  << inet_ntoa(client.sin_addr) << " "
			  << ntohs(client.sin_port) << std::endl
			  << BLUE << (message == NULL ? "\n" : message) << RESET << std::endl;
}
int Server::manageServerLoop() {
    std::vector<pollfd> poll_fds;
    pollfd server_poll_fd;

    server_poll_fd.fd = _server_socket_fd;
    server_poll_fd.events = POLLIN;

    poll_fds.push_back(server_poll_fd);

    while (1) {
        std::vector<pollfd> new_pollfds;

        if (poll((pollfd*)&poll_fds[0], (unsigned int)poll_fds.size(), -1) <= SUCCESS) {
            std::cerr << RED << "Poll error" << RESET << std::endl;
            return (FAILURE);
        }

        std::vector<pollfd>::iterator it = poll_fds.begin();
        while (it != poll_fds.end()) {
            if (it->revents & POLLIN) {
                if (it->fd == _server_socket_fd) {
                    int client_sock = acceptSocket(_server_socket_fd);
                    if (client_sock == -1) {
                        std::cerr << RED << "Accept failed" << RESET << std::endl;
                        continue;
                    }
                    if (poll_fds.size() - 1 < MAX_CLIENT_NB)
                        addClient(client_sock, new_pollfds);
                    else
                        tooManyClients(client_sock);
                    it++;
                }
                else {
                    char message[BUF_SIZE_MSG];
                    int read_count;

                    memset(message, 0, sizeof(message));
                    read_count = recv(it->fd, message, BUF_SIZE_MSG, 0);

                    if (read_count <= FAILURE) {
                        std::cerr << RED << "Recv() failed" << RESET << std::endl;
                        delClient(poll_fds, it);
                        if ((unsigned int)(poll_fds.size() - 1) == 0)
                            break;
                    }
                    else if (read_count == 0) {
                        delClient(poll_fds, it);
                        std::cout << "Disconnected\n";
                        if ((unsigned int)(poll_fds.size() - 1) == 0)
                            break;
                    }
                    else {
                        print("Recv : ", it->fd, message);
                        try {
                            parseMessage(it->fd, message);
                        }
                        catch (const std::exception& e) {
                            std::cout << "Caught exception : " << std::endl;
                            std::cerr << e.what() << std::endl;
                            delClient(poll_fds, it);
                            std::cout << "Client deleted." << std::endl;
                            if ((unsigned int)(poll_fds.size() - 1) == 0)
                                break;
                        }
                        it++;
                    }
                }
            }
            else if (it->revents & POLLERR) {
                if (it->fd == _server_socket_fd) {
                    std::cerr << RED << "Listen socket error" << RESET << std::endl;
                    return (FAILURE);
                }
                else {
                    delClient(poll_fds, it);
                    if ((unsigned int)(poll_fds.size() - 1) == 0)
                        break;
                }
            }
            else {
                it++;
            }
        }
        poll_fds.insert(poll_fds.end(), new_pollfds.begin(), new_pollfds.end());
    }
    return (SUCCESS);
}
