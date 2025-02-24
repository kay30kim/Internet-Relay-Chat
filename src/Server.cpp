#include "Server.hpp"

Server::Server() : _servinfo(NULL), _server_socket_fd(0)
{
	std::cout << YELLOW << "Server Constructor" << RESET << std::endl;
	memset(&_hints, 0, sizeof(_hints));
}

Server::~Server()
{
	std::cout << YELLOW << "Server destructor" << RESET << std::endl;
}

const char * 	Server::InvalidClientException::what (void) const throw() 
{
	return "The information given by the client are invalid.";
}

void	Server::setHints()
{
	_hints.ai_family = AF_INET;
	_hints.ai_socktype = SOCK_STREAM;
	_hints.ai_flags = AI_PASSIVE;
}

std::string	Server::getMdp() const
{ 
	return (_mdp);
}

std::map<std::string, Channel>	Server::getChannels() const
{
	return (_channels);
}

/**
 * @brief Helps set up the structs 'hints' and 'servinfo' of our Server class
 * 
 * @param port Value given by the user (e.g. "6667")
 * @return int Returns SUCCESS or FAILURE whether getaddrinfo works or not
 */
int		Server::fillServinfo(char *port)
{
	if (getaddrinfo(NULL, port, &_hints, &_servinfo) < 0)
	{
		std::cerr << RED << "Flop du addrinfo" << RESET << std::endl;
		return (FAILURE);
	}
	return (SUCCESS);
}

/**
 * @brief This function follows step by step the required function calls to launch the server:
 * 
 * 		1) socket() => get the server socket file descriptor
 * 		2) setsocktop() => enable the configuration of said socket (here, we wanted
 * 							to allow the re-use of a port if the IP address is different)
 * 		3) bind() => Associate the socket with a specific port (here, the one given by the user)
 * 		4) listen() => Wait for any incoming connections to our server socket
 * 
 * @return int 0 for SUCCESS and -1 for FAILURE
 */
int		Server::launchServer()
{
	_server_socket_fd = socket(_servinfo->ai_family, _servinfo->ai_socktype, _servinfo->ai_protocol);
	if (_server_socket_fd == FAILURE)
	{
		std::cerr << RED << "Socket failes" << RESET << std::endl;
		return (FAILURE);
	}
	int optvalue = 1; // enables the re-use of a port if the IP address is different
	if (setsockopt(_server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &optvalue, sizeof(optvalue)) == FAILURE)
	{
		std::cerr << RED << "Impossible to reuse" << RESET << std::endl;
		return (FAILURE);
	}
	if (bind(_server_socket_fd, _servinfo->ai_addr, _servinfo->ai_addrlen) == FAILURE)
	{
		std::cerr << RED << "Bind failed" << RESET << std::endl;;
		return (FAILURE);
	}
	if (listen(_server_socket_fd, BACKLOG) == FAILURE)
	{
		std::cerr << RED << "Listen failed" << RESET << std::endl;
		return (FAILURE);
	}
	return (SUCCESS);
}

void	Server::addClient(int client_socket, std::vector<pollfd> &poll_fds)
{
	pollfd	client_pollfd;
	Client	new_client(client_socket);

	client_pollfd.fd = client_socket;
	client_pollfd.events = POLLIN;
	poll_fds.push_back(client_pollfd);
	
	_clients.insert(std::pair<int, Client>(client_socket, new_client));		// insert a new nod in client map with the fd as key
	std::cout << PURPLE << "ADDED CLIENT SUCCESSFULLY" << RESET << std::endl;
}

void	Server::delClient(std::vector<pollfd> &poll_fds, std::vector<pollfd>::iterator &it)
{
	std::cout << "Deconnection of client : " << it->fd << std::endl;
	int key = it->fd;
	std::vector<pollfd>::iterator		iterator;
	for (iterator = poll_fds.begin(); iterator != poll_fds.end(); iterator++)
	{
		if (iterator->fd == it->fd)
		{
			close(it->fd);
			poll_fds.erase(iterator);
			_clients.erase(key);
			break;
		}
	}
	std::cout << CYAN << "Client deleted \nTotal Client is now: " << (unsigned int)(poll_fds.size() - 1) << RESET << std::endl;
}

/**
 * @brief Returns a dynamic Welcome version compliant with the templates below
 * 		 ":127.0.0.1 001 tmanolis :Welcome tmanolis!tmanolis@127.0.0.1\r\n"
 * 		FYI, the doc :
 * 		001    RPL_WELCOME
 *      "Welcome to the Internet Relay Network
 *       <nick>!<user>@<host>"
 */
std::string getWelcomeReply(std::map<const int, Client>::iterator &it)
{
	std::stringstream	rpl_welcome;
	std::string			host = ":127.0.0.1";
	std::string			space = " ";
	std::string			welcome = " :Welcome ";
	std::string			rpl_code = "001";
	std::string			user_id = it->second.getNickname() + "!" + it->second.getUsername() + "@" + host;
	std::string			end = "\r\n";
	
	// reset the stringstream
	rpl_welcome.str(std::string());
	// write in the stream to append everything in one line and properly terminate it with a NULL operator
	rpl_welcome << host << space << rpl_code << space << it->second.getNickname() << welcome << user_id << end << '\0';
	// convert the stream in the required std::string
	return (rpl_welcome.str());
}

std::string	cleanStr(std::string str)
{
	// Erase the space at the beginning of the str (i.e " marine sanjuan" must be "marine sanjuan")
	if (str.find(' ') != std::string::npos && str.find(' ') == 0)
		str.erase(str.find(' '), 1);
	// Erase any Carriage Returns in the str. Note : the '\n' has already be dealt with in the function SplitMessage
	if (str.find('\r') != std::string::npos)
		str.erase(str.find('\r'), 1);
	return (str);
}

void Server::fillClients(std::map<const int, Client> &client_list, int client_fd, std::vector<std::string> cmds)
{
	std::map<const int, Client>::iterator it;
	
	it = client_list.find(client_fd);
	for (size_t i = 0; i != cmds.size(); i++)
	{
		if (cmds[i].find("NICK") != std::string::npos)
		{
			cmds[i].erase(cmds[i].find("NICK"), 4);
			cmds[i] = cleanStr(cmds[i]);
			it->second.setNickname(cmds[i]);
		}
		else if (cmds[i].find("USER") != std::string::npos)
		{
			cmds[i].erase(cmds[i].find("USER "), 5);
			it->second.setUsername(cmds[i].substr(cmds[i].find(" "), cmds[i].find(" ") + 1));
			it->second.setUsername(cleanStr(it->second.getUsername()));
			it->second.setRealname(cmds[i].substr(cmds[i].find(":") + 1, cmds[i].length() - cmds[i].find(":") + 1));
		}
	}
	if (it->second.is_valid() == SUCCESS)
		send(client_fd, getWelcomeReply(it).c_str(), getWelcomeReply(it).size(), 0);
	else
		throw Server::InvalidClientException();
}

static void	splitMessage(std::vector<std::string> &cmds, std::string msg)
{
	int 		pos = 0;
	std::string	delimiter = "\n";
	std::string	substr;
	
	while ((pos = msg.find(delimiter)) != static_cast<int>(std::string::npos))
	{
		substr = msg.substr(0, pos);
		cmds.push_back(substr);
		msg.erase(0, pos + delimiter.length());
	}
}

void	Server::parseMessage(Server *server, int const client_fd, std::string message)
{
	std::vector<std::string> cmds;

	splitMessage(cmds, message);
	if (cmds[0].find("CAP LS") != std::string::npos)
	{
		fillClients(_clients, client_fd, cmds);
	}
	else
	{
		for (size_t i = 0; i != cmds.size(); i++)
			execCommand(server, client_fd, cmds[i]);
	}
}
void Server::execCommand(Server *server, int const client_fd, std::string cmd_line)
{
	std::string	validCmds[VALID_LEN] = {
		"INVITE",
		"JOIN",
		"KICK",
		"KILL",
		"LIST",
		"MDP",
		"MODE",
		"NICK",
		"PART",
		"PING",
		"PONG",
		"PRIVMSG",
		"TOPIC",
		"USER",
		"WHO",
		"WHOIS",
		"WHOWAS"
		};

	cmd_struct cmd_infos;
	int index = 0;

	parseCommand(cmd_line, cmd_infos);

	while (index < VALID_LEN)
	{
		if (cmd_infos.name == validCmds[index])
			break;
		index++;
	}

	switch (index + 1)
	{
	// case 1: invite(client_fd, cmd_infos); break;
	// case 2: join(cmd_infos); break;
	// case 3: kick(cmd_infos); break;
	// case 4: kill(cmd_infos); break;
	// case 5: list(cmd_infos); break;
	// case 6: mdp(cmd_infos); break;
	// case 7: mode(cmd_infos); break;
	// case 8: nick(cmd_infos); break;
	// case 9: part(cmd_infos); break;
	// case 10: ping(client_fd, cmd_infos); break;
	case 11: ban(server, cmd_infos); break;
	// case 12: privmsg(cmd_infos); break;
	// case 13: topic(cmd_infos); break;
	// case 14: user(cmd_infos); break;
	// case 15: who(cmd_infos); break;
	// case 16: whois(cmd_infos); break;
	// case 17: whowas(cmd_infos); break;
	default:
		std::cout << PURPLE << "This command is not supported by our services." << RESET << std::endl;
	}
}