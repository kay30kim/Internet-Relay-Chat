#include "Channel.hpp"

Channel::Channel(std::string const &channelName): _name(channelName) {}

Channel::~Channel() {}

std::string		Channel::getName() const
{
	return (_name);
}

std::map <std::string, Client>	Channel::getClientList() const
{
	return (_clientList);
}

std::vector<std::string>		Channel::getOperators() const
{
	return (_operators);
}

bool	Channel::doesClientExist(std::string &clientName)
{
	std::map <std::string, Client>::iterator it = _clientList.find(clientName);
	if (it == _clientList.end())
		return (false);
	return (true);
}

void	Channel::addClientToChannel(Client &client)
{
	this->_clientList.insert(std::pair<std::string, Client>(client.getNickname(), client));
}

void	Channel::printClientList()
{
	std::cout <<  "Here is the Client list of the Channel " << YELLOW << this->getName() << RESET << std::endl;
	for (std::map <std::string, Client>::iterator it = _clientList.begin(); \
		 it != _clientList.end(); it++)
		std::cout << it->first << std::endl;
}

/**
 * @brief Removes a Client from the ClientList of the Channel.
 * 		  Also strips this Client of the Operator privileges.
 * 
 * @param clientName 
 */
void	Channel::removeClientFromChannel(std::string &clientName)
{
	std::map <std::string, Client>::iterator it = this->_clientList.find(clientName);
	if (it != _clientList.end())
		this->_clientList.erase(it);
	
	// This function checks if the ClientName is present in the Operator list
	// and if so, deletes it.
	removeOperator(clientName); 
}

void	Channel::addToBanned(std::string &banned_name)
{
	std::vector<std::string>::iterator it;
	for (it = _banned_users.begin(); it != _banned_users.end(); it++)
	{
		if (*it == banned_name)
		{
			std::cout << banned_name << " is already banned from the channel " << getName() << std::endl;
			return ;
		}
	}
	_banned_users.push_back(banned_name);
	std::cout << RED << banned_name << " is now banned from the channel " << getName() << RESET << std::endl;
}

void	Channel::removeFromBanned(std::string &banned_name)
{
	std::vector<std::string>::iterator user;
	for (user = _banned_users.begin(); user != _banned_users.end(); user++)
	{
		if (*user == banned_name)
		{
			_banned_users.erase(user);
			std::cout << banned_name << " is not banned anymore from the channel " << getName() << std::endl;
			return ;
		}
	}
	std::cout << "No need! " << banned_name << " has never been banned from the channel " << getName() << std::endl;
}

bool	Channel::isBanned(std::string &banned_name)
{
	std::vector<std::string>::iterator user;
	if (_banned_users.empty())
		return (false);
	for (user = _banned_users.begin(); user != _banned_users.end(); user++)
	{
		if (*user == banned_name)
			return (true);
	}
	return (false);	
}

void	Channel::addFirstOperator(std::string operatorName)
{
	if (_operators.empty())
		_operators.push_back(operatorName);
}

void	Channel::removeOperator(std::string operatorName)
{
	std::vector<std::string>::iterator it;
	for (it = _operators.begin(); it != _operators.end(); it++)
	{
		if (*it == operatorName)
			_operators.erase(it);
	}
}

bool 	Channel::isOperator(std::string &operatorName)
{
	std::vector<std::string>::iterator user;
	if (_operators.empty())
		return (false);
	for (user = _operators.begin(); user != _operators.end(); user++)
	{
		if (*user == operatorName)
			return (true);
	}
	return (false);
}

void	Channel::printOperators(void)
{
	std::cout << "Print operators : \n";
	std::vector<std::string>::iterator it;
	for (it = _operators.begin(); it != _operators.end(); it++)
	{
		std::cout << *it << std::endl;
	}
}
