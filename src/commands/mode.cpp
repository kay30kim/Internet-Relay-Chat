#include "Irc.hpp"
#include "Channel.hpp"
#include "Server.hpp"
#include "Commands.hpp"

/**
 * @brief The MODE command is used to set or remove options (or modes) from a given target.
 * 
 * Parameters: <target> [<modestring> [<mode arguments>...]]
 * 
 * <modestring> starts with a plus ('+', 0x2B) or minus ('-', 0x2D) character, and is made up of the following characters:
 * '+': Adds the following mode(s).
 * '-': Removes the following mode(s).
 * 'a-zA-Z': Mode letters, indicating which modes are to be added/removed.
 * 
 * User case :
 * If <modestring> is given, the supplied modes will be applied : a MODE message will be sent to the user containing the changed modes. 
 * If one or more modes sent are not implemented on the server, 
 * the server MUST apply the modes that are implemented, and then send the ERR_UMODEUNKNOWNFLAG (501) in reply along with the MODE message.
 * 
 * Channel case :
 * syntax command : /mode <channel> <+ or -> <mode> [parameters]
 * If the user has permission to change modes on the target, the supplied modes will be applied based on the type of the mode (see below). 
 * For type A, B, and C modes, arguments will be sequentially obtained from <mode arguments>. 
 * - If a type B or C mode does not have a parameter when being set, the server MUST ignore that mode. 
 * - If a type A mode has been sent without an argument : 
 * the contents of the list MUST be sent to the user, 
 * unless it contains sensitive information the user is not allowed to access. 
 * When the server is done processing the modes, a MODE command is sent to all members of the channel containing the mode changes. 
 * Servers MAY choose to hide sensitive information when sending the mode changes.
 * 
 * Examples :
 * - +o : /mode #cool +o MEAT (MEAT becomes an operator on #cool)
 * Message to be sent to channel users
 * The irc.example.com server gave channel operator privileges to MEAT on #cool.
 * - +k : /mode #cool +k COOLKEY (protects the channel with the password COOLKEY)
 * - +s : /mode #cool +s (the channel becomes secret)
 * - multiple modes: /mode #cool +ts (the channel is in +t +s mode)
 * 
 * @param server
 * @param client_fd User sending a msg
 * @param cmd_infos Structure w/ prefix, command name and message
 * 
 * 
 * NUMERIC REPLIES:
 * User mode :
 * - ERR_NOSUCHNICK (401) : If <target> is a nickname that does not exist on the network,
 * - ERR_USERSDONTMATCH (502) : If <target> is a different nick than the user who sent the command
 * - RPL_UMODEIS (221) : If <modestring> is not given, the RPL_UMODEIS (221) numeric is sent back containing the current modes of the target user.
 * - ERR_UMODEUNKNOWNFLAG (501) : If one or more modes sent are not implemented on the server
 * 
 * 
 * Channel mode :
 * - ERR_NOSUCHCHANNEL (403) : If <target> is a channel that does not exist on the network
 * - RPL_CHANNELMODEIS (324) : If <modestring> is not given.
 * "<client> <channel> <modestring> <mode arguments>..." => Sent to a client to inform them of the currently-set modes of a channel. 
 * <channel> is the name of the channel.
 * <modestring> and <mode arguments> are a mode string and the mode arguments (delimited as separate parameters) as defined in the MODE message description.
 * Servers MAY choose to hide sensitive information such as channel keys when sending the current modes.
 * - ERR_CHANOPRIVSNEEDED (482) :  If a user does not have appropriate privileges to change modes on the target channel, the server MUST NOT process the message, and return this error
 * 
 * Useful link : https://modern.ircdocs.horse/#mode-message
 * 
 */


/*
TO BE CODED FOR THE USER: +i / +o
- Invisible User Mode :
This mode is standard, and the mode letter used for it is "+i".
If a user is set to ‘invisible’, they will not show up in commands such as WHO or NAMES unless they share a channel with the user that submitted the command. 
In addition, some servers hide all channels from the WHOIS reply of an invisible user they do not share with the user that submitted the command.

- Oper User Mode :
This mode is standard, and the mode letter used for is it "+o".
If a user has this mode, this indicates that they are a network operator.

TO BE CODED FOR THE CHANNEL: +o / +k / +s / +t
- Key Channel Mode :
This mode is standard, and the mode letter used for it is "+k".
This mode letter sets a ‘key’ that must be supplied in order to join this channel. 
If this mode is set, its’ value is the key that is required. 
Servers may validate the value (eg. to forbid spaces, as they make it harder to use the key in JOIN messages). 
If the value is invalid, they SHOULD return ERR_INVALIDMODEPARAM. However, clients MUST be able to handle any of the following:

    ERR_INVALIDMODEPARAM
    ERR_INVALIDKEY
    MODE echoed with a different key (eg. truncated or stripped of invalid characters)
    the key changed ignored, and no MODE echoed if no other mode change was valid.

If this mode is set on a channel, and a client sends a JOIN request for that channel, they must supply <key> in order for the command to succeed. 
If they do not supply a <key>, or the key they supply does not match the value of this mode, they will receive an ERR_BADCHANNELKEY (475) reply and the command will fail.

- Secret Channel Mode :
This mode is standard, and the mode letter used for it is "+s".
This channel mode controls whether the channel is ‘secret’, and does not have any value.
A channel that is set to secret will not show up in responses to the LIST or NAMES command unless the client sending the command is joined to the channel. 
Likewise, secret channels will not show up in the RPL_WHOISCHANNELS (319) numeric unless the user the numeric is being sent to is joined to that channel.

- Protected Topic Mode :
This mode is standard, and the mode letter used for it is "+t".
This channel mode controls whether channel privileges are required to set the topic, and does not have any value.
If this mode is enabled, users must have channel privileges such as halfop or operator status in order to change the topic of a channel. 
In a channel that does not have this mode enabled, anyone may set the topic of the channel using the TOPIC command.

*/

struct mode_struct
{
	std::string	target;
	std::string	mode;
	std::string	params;
};

static void	broadcastToAllChannelMembers(Server *server, Channel &channel, std::string reply)
{
	std::map<std::string, Client>::iterator member = channel.getClientList().begin();
	
	while (member != channel.getClientList().end())
	{
		addToClientBuffer(server, member->second.getClientFd(), reply);
		member++;
	}
}

static void	fillModeInfos(mode_struct &mode_infos, std::string command)
{	
	size_t						pos;

	// TARGET
	pos = command.find(" ");
	if (pos == std::string::npos)
	{
		mode_infos.target = command.substr(0);
		return ;
	}
	else
	{
		mode_infos.target = command.substr(0, pos);
		command.erase(0, pos + 1);
	}

	// MODE
	pos = command.find(" ");
	if (pos == std::string::npos)
	{
		mode_infos.mode = command.substr(0);
		return ;
	}
	else
	{
		mode_infos.mode = command.substr(0, pos);
		command.erase(0, pos + 1);
	}

	// PARAM
	mode_infos.params = command.substr(0);
}

static void	operatorChannelMode(Server *server, mode_struct mode_infos, int const client_fd, std::string str)
{
	std::map<const int, Client>::iterator it_client = server->getClients().find(client_fd);
	std::map<std::string, Channel>::iterator it_channel_target = server->getChannels().find(mode_infos.target);
	
	if (mode_infos.params.empty() == true)
		return ;

	// check that the parameter is an IRC user, then a user of the channel
	std::map<const int, Client>::iterator it_user_target = server->getClients().begin();
    while (it_user_target != server->getClients().end())
    {
    	if (it_user_target->second.getNickname() == mode_infos.params)
        	break;
    	it_user_target++;
    }
	if (it_user_target == server->getClients().end())
	{
		addToClientBuffer(server, client_fd, ERR_NOSUCHNICK(it_client->second.getNickname(), mode_infos.params));
		addToClientBuffer(server, client_fd, ERR_USERNOTINCHANNEL(it_client->second.getNickname(), mode_infos.params, mode_infos.target));
		return ;
	}

	if (it_channel_target->second.getClientList().find(mode_infos.params) == it_channel_target->second.getClientList().end())
	{
		addToClientBuffer(server, client_fd, ERR_USERNOTINCHANNEL(it_client->second.getNickname(), mode_infos.params, mode_infos.target));
		return ;
	}

	std::vector<std::string>::iterator it;
	for (it = it_channel_target->second.getOperators().begin(); it != it_channel_target->second.getOperators().end(); it++)
	{
		if (*it == mode_infos.params)
			break;
	}

	if (str[0] == '+')
	{
		if (it != it_channel_target->second.getOperators().end())
			return ;

		it_channel_target->second.getOperators().push_back(mode_infos.params);
		broadcastToAllChannelMembers(server, it_channel_target->second, MODE_CHANNELMSGWITHPARAM(mode_infos.target, "+o", mode_infos.params));
	}
	else
	{
		if (it == it_channel_target->second.getOperators().end())
			return ;

		it_channel_target->second.getOperators().erase(it);
		broadcastToAllChannelMembers(server, it_channel_target->second, MODE_CHANNELMSGWITHPARAM(mode_infos.target, "-o", mode_infos.params));
	}
}

static void	topicChannelMode(Server *server, mode_struct mode_infos, int const client_fd, std::string mode_str)
{
	std::cout << "I am in mode t" << std::endl;
	std::map<const int, Client>::iterator it_client = server->getClients().find(client_fd);
	(void)it_client;
	std::map<std::string, Channel>::iterator it_channel_target = server->getChannels().find(mode_infos.target);

	size_t pos = it_channel_target->second.getMode().find("t");
	if (mode_str[0] == '+')
	{
		if (pos != std::string::npos)
			return;
		it_channel_target->second.addMode("t");
		broadcastToAllChannelMembers(server, it_channel_target->second, MODE_CHANNELMSG(mode_infos.target, "+t"));
	}
	else
	{
		if (pos == std::string::npos)
			return;
		it_channel_target->second.removeMode("t");
		broadcastToAllChannelMembers(server, it_channel_target->second, MODE_CHANNELMSG(mode_infos.target, "-t"));
	}
}

static void	secretChannelMode(Server *server, mode_struct mode_infos, int const client_fd, std::string mode_str)
{
	std::cout << "I am in mode s" << std::endl;
	std::map<const int, Client>::iterator it_client = server->getClients().find(client_fd);
	(void)it_client;
	std::map<std::string, Channel>::iterator it_channel_target = server->getChannels().find(mode_infos.target);

	size_t pos = it_channel_target->second.getMode().find("s");
	if (mode_str[0] == '+')
	{
		if (pos != std::string::npos)
			return;
		it_channel_target->second.addMode("s");
		broadcastToAllChannelMembers(server, it_channel_target->second, MODE_CHANNELMSG(mode_infos.target, "+s"));
	}
	else
	{
		if (pos == std::string::npos)
			return;
		it_channel_target->second.removeMode("s");
		broadcastToAllChannelMembers(server, it_channel_target->second, MODE_CHANNELMSG(mode_infos.target, "-s"));
	}
}

static bool isAlpha(std::string str) 
{
    for (size_t i = 0; i < str.length(); i++) 
	{
        if (!isalpha(str[i])) 
            return (false);
    }
    return (true);
}

static void	keyChannelMode(Server *server, mode_struct mode_infos, int const client_fd, std::string mode_str)
{
	std::cout << "I am in mode k" << std::endl;
	std::map<const int, Client>::iterator it_client = server->getClients().find(client_fd);
	std::map<std::string, Channel>::iterator it_channel_target = server->getChannels().find(mode_infos.target);


	size_t pos = it_channel_target->second.getMode().find("k");
	if (mode_infos.params.empty() == true)
		return ;
	
	if (mode_str[0] == '+')
	{
		if (pos != std::string::npos)
			return;
		if (isAlpha(mode_infos.params) == false) 
		{
			addToClientBuffer(server, client_fd, ERR_INVALIDMODEPARAM(it_client->second.getNickname(), mode_infos.target, "+k", mode_infos.params));
			return ;
		}
		it_channel_target->second.addMode("k");
		it_channel_target->second.setChannelPassword(mode_infos.params);
		broadcastToAllChannelMembers(server, it_channel_target->second, MODE_CHANNELMSGWITHPARAM(mode_infos.target, "+k", mode_infos.params));
	}
	else
	{
		if (pos == std::string::npos)
			return;
		it_channel_target->second.removeMode("k");
		it_channel_target->second.removeChannelPassword();
		broadcastToAllChannelMembers(server, it_channel_target->second, MODE_CHANNELMSGWITHPARAM(mode_infos.target, "-k", mode_infos.params));
	}
}

static void	changeChannelMode(Server *server, mode_struct mode_infos, int const client_fd)
{
	std::map<const int, Client>::iterator it_client = server->getClients().find(client_fd);
	std::map<std::string, Channel>::iterator it_channel_target = server->getChannels().find(mode_infos.target);

	(void)it_client;
	(void)it_channel_target;

	std::vector<std::string> vector_modes;
	for (int i = 1; mode_infos.mode[i] != '\0'; i++)
	{
		if (mode_infos.mode[i] == '+' || mode_infos.mode[i] == '-')
		{
			vector_modes.push_back(mode_infos.mode.substr(0, i));
			mode_infos.mode.erase(0, i);
			i = 0;
		}
	}
	vector_modes.push_back(mode_infos.mode.substr(0));
	
	// apply modes
	for (std::vector<std::string>::iterator it = vector_modes.begin(); it != vector_modes.end(); it++)
	{
		std::string str = *it;
		if (str.find("o") != std::string::npos)
			operatorChannelMode(server, mode_infos, client_fd, str);
		if (str.find("t") != std::string::npos)
			topicChannelMode(server, mode_infos, client_fd, str);
		if (str.find("s") != std::string::npos)
			secretChannelMode(server, mode_infos, client_fd, str);
		if (str.find("k") != std::string::npos)
			keyChannelMode(server, mode_infos, client_fd, str);
	}
}

static void	modeForChannel(Server *server, mode_struct mode_infos, int const client_fd)
{
	// syntax command : /mode <channel> <+ ou -> <mode> [parametres]
	std::map<const int, Client>::iterator it_client = server->getClients().find(client_fd);

	mode_infos.target.erase(0,1); // erase '#'
	std::cout << "CHANNEL - target : |" << mode_infos.target << "|" << std::endl;

	std::map<std::string, Channel>::iterator it_channel_target = server->getChannels().find(mode_infos.target);
	if (it_channel_target == server->getChannels().end())
	{
		addToClientBuffer(server, client_fd, ERR_NOSUCHCHANNEL(it_client->second.getNickname(), mode_infos.target));
		return ;
	}

	// If <mode> is not given
	if (mode_infos.mode.empty() == true)
	{
		if (it_channel_target->second.getClientList().find(it_client->second.getNickname()) != it_channel_target->second.getClientList().end() \
		&& it_channel_target->second.getChannelPassword().empty() == false) 
			addToClientBuffer(server, client_fd, RPL_CHANNELMODEISWITHKEY(it_client->second.getNickname(), mode_infos.target, it_channel_target->second.getMode(), it_channel_target->second.getChannelPassword()));
		else
			addToClientBuffer(server, client_fd, RPL_CHANNELMODEIS(it_client->second.getNickname(), mode_infos.target, it_channel_target->second.getMode()));
		return ;
	}

	std::vector<std::string>::iterator it;
	for (it = it_channel_target->second.getOperators().begin(); it != it_channel_target->second.getOperators().end(); it++)
	{
		if (*it == it_client->second.getNickname())
			break;
	}
	if (it == it_channel_target->second.getOperators().end())
	{
		addToClientBuffer(server, client_fd, ERR_CHANOPRIVSNEEDED(it_client->second.getNickname(), mode_infos.target));
		return ;
	}
	if (mode_infos.mode[0] == '+' || mode_infos.mode[0] == '-')
		changeChannelMode(server, mode_infos, client_fd);
}

static void	modeForUser(Server *server, mode_struct mode_infos, int const client_fd)
{
	std::map<const int, Client>::iterator it_client = server->getClients().find(client_fd);

	std::map<const int, Client>::iterator it_user_target = server->getClients().begin();
    while (it_user_target != server->getClients().end())
    {
    	if (it_user_target->second.getNickname() == mode_infos.target)
        	break;
    	it_user_target++;
    }
	if (it_user_target == server->getClients().end())
	{
		addToClientBuffer(server, client_fd, ERR_NOSUCHNICK(it_client->second.getNickname(), mode_infos.target));
		return ;
	}
	//  If <target> is a different nick than the user who sent the command
	if (it_user_target->second.getNickname() != it_client->second.getNickname())
	{
		addToClientBuffer(server, client_fd, ERR_USERSDONTMATCH(it_client->second.getNickname()));
		return ;
	}

	// If <mode> is not given
	if (mode_infos.mode.empty() == true)
		addToClientBuffer(server, client_fd, RPL_UMODEIS(it_client->second.getNickname(), it_client->second.getMode()));

	if (mode_infos.mode[0] == '+' || mode_infos.mode[0] == '-')
	{
		std::string::iterator pos = mode_infos.mode.begin();
		while (pos != mode_infos.mode.end())
		{
			if (*pos == '+')
			{
				pos++;
				while (*pos != '+' && *pos != '-' && pos != mode_infos.mode.end())
				{
					if (*pos == 'i')
					{
						if (it_user_target->second.getMode().find("i") == std::string::npos)
						{
							it_user_target->second.addMode("i");
							addToClientBuffer(server, client_fd, MODE_USERMSG(it_client->second.getNickname(), "+i"));
						}
					}
					pos++;
				}
			}
			if (*pos == '-')
			{
				pos++;
				while (*pos != '+' && *pos != '-' && pos != mode_infos.mode.end())
				{
					if (*pos == 'i')
					{
						if (it_user_target->second.getMode().find("i") != std::string::npos)
						{
							it_user_target->second.removeMode("i");
							addToClientBuffer(server, client_fd, MODE_USERMSG(it_client->second.getNickname(), "-i"));
						}
					}
					if (*pos == 'o')
					{
						if (it_user_target->second.getMode().find("o") != std::string::npos)
						{
							it_user_target->second.removeMode("o");
							addToClientBuffer(server, client_fd, MODE_USERMSG(it_client->second.getNickname(), "-o"));
						}
					}
					pos++;
				}
			}
		}
		if (mode_infos.mode.find("O") != std::string::npos || mode_infos.mode.find("r") != std::string::npos || mode_infos.mode.find("w") != std::string::npos)
			addToClientBuffer(server, client_fd, ERR_UMODEUNKNOWNFLAG(it_client->second.getNickname()));
	}
}

void	modeFunction(Server *server, int const client_fd, cmd_struct cmd_infos)
{
	mode_struct	mode_infos;
	
	std::cout << "\nMessage : |" << cmd_infos.message << "|" << std::endl;
	cmd_infos.message.erase(0,1);

	fillModeInfos(mode_infos, cmd_infos.message);
	
	std::cout << "target : |" << mode_infos.target << "|" << std::endl;
	std::cout << "mode : |" << mode_infos.mode << "|" << std::endl;
	std::cout << "params : |" << mode_infos.params << "|" << std::endl;

	if (mode_infos.target[0] == '#') // channel case
		modeForChannel(server, mode_infos, client_fd);
	else // user case
		modeForUser(server, mode_infos, client_fd);
}