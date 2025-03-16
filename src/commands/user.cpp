#include "Irc.hpp"
#include "Channel.hpp"
#include "Server.hpp"
#include "Commands.hpp"

std::string	findUsername(std::string msg);
std::string	findRealname(std::string msg);

/**
 * @brief The USER command is used at the beginning of a connection to specify 
 *        the username and real name of a new user.
 * 		
 *  The minimum length of <username> is 1, i.e., it MUST NOT be empty. 
 *  If it is empty, the server SHOULD reject the command with ERR_NEEDMOREPARAMS.
 * 
 *  If a client tries to send the USER command after they have already completed
 *  registration with the server, the ERR_ALREADYREGISTERED reply should be sent
 *  and the attempt should fail.
 * 	
 *  SYNTAX : USER <username> 0 * <realname>
 * 
 *  Numeric Replies:
 *      ERR_NEEDMOREPARAMS (461)
 *      ERR_ALREADYREGISTERED (462)
 * 
 *  Examples:
 *  [Client] USER marine 0 * :Marine Sanjuan
 *  => Username is marine, Realname is Marine Sanjuan
 * 
 *  [Client] USER msanjuan msanjuan localhost :Marine SANJUAN
 *  => Username is msanjuan, Realname is Marine Sanjuan
 */
void	user(Server *server, int const client_fd, cmd_struct cmd_infos)
{
	// Parse the command line
	Client&		client		= retrieveClient(server, client_fd);
	std::string	username	= findUsername(cmd_infos.message);
	std::string realname	= findRealname(cmd_infos.message);

	// DEBUG (commented out for clarity)
	// std::cout << "Username is |" << username << "|" << std::endl;
	// std::cout << "Realname is |" << realname << "|" << std::endl;

	if (username.empty() || realname.empty())
		addToClientBuffer(server, client_fd, ERR_NEEDMOREPARAMS(client.getNickname(), cmd_infos.name));
	else if (client.isRegistrationDone() == true)
		addToClientBuffer(server, client_fd, ERR_ALREADYREGISTERED(client.getNickname()));
	else
	{
		client.setUsername(username);
		client.setRealname(realname);
	}
}

// Test cases (for validation):
// " msanjuan msanjuan localhost :Marine SANJUAN" => valid
// ""                              => error
// " "                             => error
// " msanjuan"                     => error
std::string	findUsername(std::string msg)
{
	std::string user;
	user.clear();

	if (!msg.empty() && msg[0] == ' ')
		msg.erase(0, 1);
	if (!msg.empty())
	{
		if (msg.find_first_of(' ') != std::string::npos)
			user.insert(0, msg, 0, msg.find(' '));
		else
			user.insert(0, msg, 0, std::string::npos);
	}
	return (user);
}

// Test cases (for validation):
// " msanjuan msanjuan localhost :Marine SANJUAN" => valid
// ""                              => error
// " "                             => error
// " msanjuan 0 * "                => error
// There must be at most 4 arguments! So either 3 spaces or nothing.
std::string	findRealname(std::string msg)
{
	std::string real;
	real.clear();

	if (!msg.empty() && msg[0] == ' ')
		msg.erase(0, 1);
	if (msg.empty())
		return (real);
	
	int space_count = 0;
	int	begin_pos = 0;
	for (size_t i = 0; i < msg.size(); i++)
	{
		if (msg[i] == ' ')
			space_count++;
		if (space_count == 3)
		{
			begin_pos = i;
			break;
		}
	}
	if (space_count >= 3)
		real.insert(0, msg, begin_pos + 1, std::string::npos); 
	if (!real.empty() && real[0] == ':')
		real.erase(0, 1);
	return (real);
}
