#include "Client.hpp"

Client::Client(int client_fd)
: _client_fd(client_fd), _connexion_password(false), _registrationDone(false), _welcomeSent(false), _hasAllInfo(false)
{
	std::cout << YELLOW << "Client constructor for Client #" << client_fd << RESET << std::endl;
}

Client::~Client()
{
	std::cout << YELLOW << "Client destructor" << RESET << std::endl;
}

int				Client::getClientFd() const { return (_client_fd); }
std::string&	Client::getNickname()  		{ return (_nickname); }
std::string&	Client::getOldNickname()  	{ return (_old_nickname); }
std::string 	Client::getUsername() const { return (_fullname); }
std::string		Client::getRealname() const { return (_realname); }

bool&			Client::getConnexionPassword()	{ return (_connexion_password); }
bool&			Client::isRegistrationDone() 	{ return (_registrationDone); }
bool&			Client::isWelcomeSent()			{ return (_welcomeSent); }
bool&			Client::hasAllInfo() 			{ return (_hasAllInfo); }

void	Client::setNickname(std::string const &nickname)
{
	// If the nickname has more than 9 characters, it must be truncated
	_nickname = (_nickname.size() > 9) ? nickname.substr(0, 9) : nickname;
}

void	Client::setOldNickname(std::string const &nickname)
{
	_old_nickname = nickname;
}

void	Client::setUsername(std::string const &username)
{
	_fullname = username;
}

void	Client::setRealname(std::string const &realname)
{
	_realname = realname;
}

void	Client::setConnexionPassword(bool boolean)
{
	_connexion_password = boolean;
}

void	Client::setRegistrationDone(bool boolean)
{
	_registrationDone = boolean;
}

void	Client::setWelcomeSent(bool boolean)
{
	_welcomeSent = boolean;
}

void	Client::sethasAllInfo(bool boolean)
{
	_hasAllInfo = boolean;
}

void	Client::printClient()const
{
	std::cout << "Print client" << std::endl;
	std::cout << YELLOW << "fd: " << _client_fd << "\n" \
			<< "nickname: " << _nickname << "\n" \
			<< "Fullname: " << _fullname << "\n"
			<< "Real name: " << _realname << RESET << "\n";

}

int	Client::is_valid() const
{
	if (_fullname.empty())
		return (FAILURE);
	if (_nickname.empty())
		return (FAILURE);
	if (_realname.empty())
		return (FAILURE);
	if (_connexion_password == false)
		return (FAILURE);
	return (SUCCESS);
}