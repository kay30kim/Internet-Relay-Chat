#include "Server.hpp"
#include "Commands.hpp"
#include "LagCompensation.hpp"

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

void Server::setHints()
{
	_hints.ai_family = AF_INET;
	_hints.ai_socktype = SOCK_STREAM;
	_hints.ai_flags = AI_PASSIVE;
}

std::string 					Server::getPort()	  const { return (_port); }

std::string 					Server::getPassword() const { return (_password); }

std::map<std::string, Channel>&	Server::getChannels()		{ return (_channels); }

std::map<const int, Client>&	Server::getClients()		{ return (_clients); }

void							Server::setPassword(std::string new_pwd)
{
	_password = new_pwd;
}

/**
 * @brief Helps set up the structs 'hints' and 'servinfo' of our Server class
 *
 * @param port Value given by the user (e.g. "6667")
 * @return int Returns SUCCESS or FAILURE whether getaddrinfo works or not
 */
int Server::fillServinfo(char *port)
{
	if (getaddrinfo(NULL, port, &_hints, &_servinfo) < 0)
	{
		std::cerr << RED << "Fail addressinfo" << RESET << std::endl;
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
int Server::launchServer()
{
	_server_socket_fd = socket(_servinfo->ai_family, _servinfo->ai_socktype, _servinfo->ai_protocol);
	if (_server_socket_fd == FAILURE)
	{
		std::cerr << RED << "Socket failed" << RESET << std::endl;
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
		std::cerr << RED << "Bind failed" << RESET << std::endl;
		return (FAILURE);
	}
	if (listen(_server_socket_fd, BACKLOG) == FAILURE)
	{
		std::cerr << RED << "Listen failed" << RESET << std::endl;
		return (FAILURE);
	}
	return (SUCCESS);
}

void Server::addClient(int client_socket, std::vector<pollfd> &poll_fds)
{
	pollfd client_pollfd;
	Client new_client(client_socket);

	client_pollfd.fd = client_socket;
	client_pollfd.events = POLLIN;
	poll_fds.push_back(client_pollfd);

	_clients.insert(std::pair<int, Client>(client_socket, new_client)); // insert a new nod in client map with the fd as key
	std::cout << PURPLE << "ADDED CLIENT SUCCESSFULLY" << RESET << std::endl;
}

void Server::delClient(std::vector<pollfd> &poll_fds, std::vector<pollfd>::iterator &it)
{
	std::cout << "Client disconnected : " << it->fd << std::endl;
	int key = it->fd;
	std::vector<pollfd>::iterator iterator;
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
	if (str.find(' ') != std::string::npos && str.find(' ') == 0)
		str.erase(str.find(' '), 1);
	if (str.find('\r') != std::string::npos)
		str.erase(str.find('\r'), 1);
	return (str);
}

void Server::fillClients(std::map<const int, Client> &client_list, int client_fd, std::string cmd) {
    std::map<const int, Client>::iterator it = client_list.find(client_fd);
    
    if (it == client_list.end()) {
        std::cerr << RED << "Client not found in list!" << RESET << std::endl;
        return;
    }

    std::cout << "Processing command: " << cmd << std::endl;

    if (cmd.find("NICK") != std::string::npos) {
        cmd.erase(cmd.find("NICK"), 4);
        cmd = cleanStr(cmd);
        std::cout << "Setting nickname: " << cmd << std::endl;
        it->second.setNickname(cmd);
    } 
    else if (cmd.find("USER") != std::string::npos) {
        std::cout << "Processing USER command: " << cmd << std::endl;

        std::vector<std::string> tokens;
        std::stringstream ss(cmd);
        std::string token;

        while (ss >> token) {
            tokens.push_back(token);
        }

        if (tokens.size() < 5) {
            std::cerr << "Invalid USER command format!" << std::endl;
            return;
        }

        it->second.setUsername(tokens[1]); // USER <username>
        it->second.setRealname(tokens[4].substr(1)); // Realname starts after ':'

        std::cout << "Setting username: " << it->second.getUsername() << std::endl;
        std::cout << "Setting realname: " << it->second.getRealname() << std::endl;
    }
    std::cout << "Client state - Nick: " << it->second.getNickname()
              << ", User: " << it->second.getUsername()
              << ", Realname: " << it->second.getRealname() << std::endl;
}

static void splitMessage(std::vector<std::string> &cmds, std::string msg)
{
	int pos = 0;
	std::string delimiter = "\n";
	std::string substr;

	while ((pos = msg.find(delimiter)) != static_cast<int>(std::string::npos))
	{
		substr = msg.substr(0, pos);
		cmds.push_back(substr);
		msg.erase(0, pos + delimiter.length());
	}
}

void Server::parseMessage(int client_fd, std::string message) {
    std::cout << "Logging message for LagCompensation: " << message << std::endl;
    LagCompensation::getInstance().logClientMessage(client_fd, message);

    std::vector<std::string> cmds;
    std::deque<TickRecord> valid_msgs = LagCompensation::getInstance().getValidMessages(client_fd);

    std::cout << "Valid messages count: " << valid_msgs.size() << std::endl;

    for (const auto& record : valid_msgs) {
        std::cout << "Processing message: " << record.message << std::endl;
        splitMessage(cmds, record.message);
    }

    std::map<const int, Client>::iterator it = _clients.find(client_fd);
    if (it == _clients.end()) {
        std::cerr << RED << "Client not found" << RESET << std::endl;
        return;
    }

    for (size_t i = 0; i < cmds.size(); i++) {
        std::cout << "Processing command: " << cmds[i] << std::endl;
        if (!it->second.isRegistrationDone()) {
            if (!it->second.hasAllInfo()) {
                fillClients(_clients, client_fd, cmds[i]);
                if (cmds[i].find("USER") != std::string::npos) {
                    it->second.hasAllInfo() = true;
                }
            }
            if (it->second.hasAllInfo() && !it->second.isWelcomeSent()) {
                if (it->second.is_valid() == SUCCESS) {
                    send(client_fd, getWelcomeReply(it).c_str(), getWelcomeReply(it).size(), 0);
                    it->second.isWelcomeSent() = true;
                    it->second.isRegistrationDone() = true;
                } else {
                    throw Server::InvalidClientException();
                }
            }
        } else {
            execCommand(client_fd, cmds[i]);
        }
    }
}

void Server::execCommand(int const client_fd, std::string cmd_line)
{
	std::string	validCmds[VALID_LEN] = {
		"INVITE",
		"JOIN",
		"KICK",
		"KILL",
		"LIST",
		"MODE",
		"NICK",
		"PART",
		"PING",
		"OPER",
		"PRIVMSG",
		"QUIT",
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
	case 1: invite(this, client_fd, cmd_infos); break;
	case 2: join(this, client_fd, cmd_infos); break;
	// case 3: kick(this, cmd_infos); break;
	// case 4: kill(cmd_infos); break;
	case 5: list(this, client_fd, cmd_infos); break;
	// case 6: mode(cmd_infos); break;
	case 7: nick(this, client_fd, cmd_infos); break;
	// case 8: part(cmd_infos); break;
	case 9: ping(client_fd, cmd_infos); break;
	// case 10: oper(this, cmd_infos); break;
  // case 11: privmsg(cmd_infos); break;
	// case 12: quit(this, cmd_infos); break;
	case 13: topic(this, client_fd, cmd_infos); break;
	// case 14: user(cmd_infos); break;
	// case 15: who(cmd_infos); break;
	// case 16: whois(cmd_infos); break;
	// case 17: whowas(cmd_infos); break;
	default:
		std::cout << PURPLE << "This command is not supported by our services." << RESET << std::endl;
	}
}

void Server::addChannel(std::string &channelName)
{
	std::map<std::string, Channel>::iterator it = _channels.find(channelName);
	if (it != _channels.end())
	{
		std::cout << "Channel already exists, choose an other name\n";
		return ;
	}
	Channel	channel(channelName);
	_channels.insert(std::pair<std::string, Channel>(channel.getName(), channel));
}

void Server::addClientToChannel(std::string &channelName, Client &client)
{
	std::map<std::string, Channel>::iterator it;
	it = _channels.find(channelName);
	std::string client_nickname = client.getNickname();
	if (it->second.doesClientExist(client_nickname) == false)
	{
		it->second.addClientToChannel(client);
		std::cout << "Client successfully joined the channel" << channelName << "!" << std::endl;
	}
	else 
		std::cout << YELLOW << client.getNickname() << "already here\n" << RESET;
}
