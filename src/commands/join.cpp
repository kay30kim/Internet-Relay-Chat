#include "Irc.hpp"
#include "Channel.hpp"
#include "Server.hpp"

std::string		getChannelName(std::string msg_to_parse);
void			addChannel(Server server, std::string const &channelName);
void			addClientToChannel(Server server, std::string &channelName, Client &client);
void			printChannel(Server server, std::string &channelName);

/**
 * @brief The JOIN command indicates that the client wants to join the given channel(s), 
 * 	each channel using the given key for it. The server receiving the command checks 
 * 	whether or not the client can join the given channel, and processes the request. 
 * 
 * 	While a client is joined to a channel, they receive all relevant information about 
 * 	that channel including the JOIN, PART, KICK, and MODE messages affecting the channel. 
 * 	They receive all PRIVMSG and NOTICE messages sent to the channel, and they also 
 * 	receive QUIT messages from other clients joined to the same channel (to let them 
 * 	know those users have left the channel and the network). 
 * 
 *  Numeric Replies:
 *  
 *  ERR_NEEDMOREPARAMS (461)
 *  ERR_NOSUCHCHANNEL (403)
 *  ERR_TOOMANYCHANNELS (405)
 *  ERR_BADCHANNELKEY (475)
 *  ERR_BANNEDFROMCHAN (474)
 *  ERR_CHANNELISFULL (471)
 *  ERR_INVITEONLYCHAN (473)
 *  ERR_BADCHANMASK (476)
 *  RPL_TOPIC (332)
 *  RPL_TOPICWHOTIME (333)
 *  RPL_NAMREPLY (353)
 *  RPL_ENDOFNAMES (366)
 *  
 *  Examples:
 * 	[CLIENT]  JOIN #foobar
 *  [SERVER] ; join channel #foobar.
 * 
 * 	[CLIENT]  JOIN #foo,#bar fubar,foobar
 * 	[SERVER]; join channel #foo using key "fubar" and channel #bar using key "foobar".
 */
void	join(Server server, int const client_fd, cmd_struct cmd_infos)
{
	std::string channelName = getChannelName(cmd_infos.message);

	std::map<const int, Client>::iterator it_client = client_list.find(client_fd);
	Client client = it_client->second;

	std::map<std::string, Channel>			 channels = server.getChannels();
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	if (it == channels.end())
		addChannel(server, channelName);
	
	std::string client_nickname = client.getNickname();
	if (it->second.isBanned(client_nickname) == SUCCESS) {
		std::cout << client.getNickname() << " is banned from " << channelName << std::endl; 
		return ;
	} 
	else {
		addClientToChannel(server, channelName, client);
		it->second.addFirstOperator(client.getNickname());
	}
}

std::string	getChannelName(std::string msg_to_parse)
{
	std::cout << "The msg_to_parse looks like this : |" << msg_to_parse << "|" << std::endl;
	// Expected output : | #foobar|
	// Expected output 2 : | #foo,#bar fubar,foobar|

	// Logique pour l'output 2 : on erase les channels (avec leur keys quand elles en ont) au fur et à mesure qu'on join

	std::string channel_name;
	for (size_t i = 0; i < msg_to_parse.size(); i++)
	{
		if (isalpha(msg_to_parse[i]))
			channel_name.append(msg_to_parse[i]);
	}
	std::cout << "The channel name is : |" << channel_name << "|" << std::endl;
	return (channel_name);
}

void	addChannel(Server server, std::string const &channelName)
{
	// check if channel already exists.
	std::map<std::string, Channel>			 channels = server.getChannels();
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	if (it != channels.end())
	{
		std::cout << "Channel already exists, choose an other name\n";
		return ;
	}
	Channel	channel(channelName);
	channels.insert(std::pair<std::string, Channel>(channelName, channel));
	// std::cout << RED << "Channel added: " << channelName << RESET << std::endl;
}

void	addClientToChannel(Server server, std::string &channelName, Client &client)
{
	std::map<std::string, Channel>			 channels = server.getChannels();
	std::map<std::string, Channel>::iterator it;
	it = channels.find(channelName);
	std::string client_nickname = client.getNickname();
	if (it->second.doesClientExist(client_nickname) == false) // BUG 
	{
		it->second.addClientToChannel(client);
	}
	else 
		std::cout << YELLOW << client.getNickname() << "already here\n" << RESET;
}

void	printChannel(Server server, std::string &channelName)
{
	std::map<std::string, Channel>			 	channels = server.getChannels();
	std::map<std::string, Channel> 				tmp;
	std::map<std::string, Channel>::iterator	it = channels.find(channelName);
	it->second.printClientList();
}
