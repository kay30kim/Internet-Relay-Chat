#include "Irc.hpp"
#include "Channel.hpp"
#include "Server.hpp"
#include "Commands.hpp"

/**
 * @brief The QUIT command is used to terminate a client’s connection to the server. 
 *  The server acknowledges this by replying with an ERROR message and closing 
 *  the connection to the client.
 * 	
 *  Parameters: [<reason>]
 * 
 *  Example: [CLIENT] QUIT :Gone to have lunch   
 *           [SERV] ;Client exiting from the network
 * 
 *  Source: https://modern.ircdocs.horse/#quit-message
 */
void	quit(Server *server, int const client_fd, cmd_struct cmd_infos)
{
	Client 		&client = retrieveClient(server, client_fd);
 	std::string	reason	= getReason(cmd_infos.message);
 
 	sendServerRpl(client_fd, RPL_ERROR(user_id(client.getNickname(), client.getUsername()), reason));
 	sendServerRpl(client_fd, RPL_QUIT(user_id(client.getNickname(), client.getUsername()), reason));
}