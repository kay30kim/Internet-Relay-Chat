#include "Irc.hpp"
#include "Channel.hpp"
#include "Server.hpp"
#include "Commands.hpp"

/**
 * @brief The KICK command can be used to request the forced removal of a user 
 *  from a channel. It causes the <user> to be removed from the <channel> by force. 
 *  If no comment is given, the server SHOULD use a default message instead.
 * 
 *  Parameters: <channel> <user> *( "," <user> ) [<comment>]
 * 
 * Numeric Replies:
 * 
 * ERR_NEEDMOREPARAMS (461)
 * ERR_NOSUCHCHANNEL (403)
 * ERR_CHANOPRIVSNEEDED (482)
 * ERR_USERNOTINCHANNEL (441)
 * ERR_NOTONCHANNEL (442)
 * 
 * Example:
 * Client's request : KICK #Finnish John :Speaking English
 * 
 * Server's Response: " Command to kick John from #Finnish using "Speaking English" 
 * 						as the reason (comment)."
 *                                  
 * @param server 
 * @param cmd_infos Structure w/ prefix, command name and message
 */
void	kick(Server server, cmd_struct cmd_infos)
{
	// TODO: Implement parsing of command.message to extract operator name, channelName, and clientName
	std::string operator_name;
	std::string channel_name;
	std::string client_name;

	std::map<std::string, Channel>::iterator it;
	it = server.getChannels().find(channel_name); // TODO: Handle the case where the channel does not exist
	if (it->second.doesClientExist(client_name) == true)
	{
		if (it->second.isOperator(operator_name) == false)
		{
			std::cout << "[ERR 482] " << operator_name << " is not an operator on " << channel_name << std::endl;
			return ;
		}
		it->second.removeClientFromChannel(client_name);
		std::cout << client_name << " has been kicked from " << channel_name << " by " << operator_name << std::endl; 
	}
	// TODO: Handle ERR 441, 442 if the user is not in the channel or does not exist
	// NOTE: We print the output of our action on the server side, but we should also SEND a message to the client
}
