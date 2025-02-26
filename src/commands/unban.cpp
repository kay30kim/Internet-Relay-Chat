#include "Irc.hpp"
#include "Server.hpp"
#include "Channel.hpp"

void	unban(Server server, cmd_struct cmd_infos)
{
	std::string operator_name;
	std::string channel_name;
	std::string name_to_unban;

	std::map<std::string, Channel>::iterator it;
	it = server.getChannels().find(channel_name);
	if (it->second.doesClientExist(name_to_unban) == true)
	{
		if (it->second.isOperator(operator_name) == false)
		{
			std::cout << operator_name << " is not admin on " << channel_name << std::endl;
			return ;
		}
		it->second.removeFromBanned(name_to_unban);
		std::cout << name_to_unban << " has been unbanned from " << channel_name << " by " << operator_name <<std::endl; 
	}	
}