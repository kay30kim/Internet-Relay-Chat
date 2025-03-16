#include "Irc.hpp"
#include "Commands.hpp"

/**
 * @brief The PING command is used to check if the other side of the connection 
 *        is still active and/or to measure connection latency.
 * 
 *        When receiving a PING message, clients or servers must respond 
 *        with a PONG message containing the same <token> value. 
 * 
 *        Numeric replies:
 *        461 - ERR_NEEDMOREPARAMS -  "<client> <command> :Not enough parameters" 
 *              > Handled by IRSSI
 *        409 - ERR_NOORIGIN - "<client> :No origin specified" (when <token> is empty) 
 *              > Handled by IRSSI
 * 
 *        IRSSI:
 *        In this reference client, sending a PING to a server sends a CTCP request
 *        to a nickname or a channel. Using the character '*' pings every user in a channel.
 *        Syntax: PING [<nick> | <channel> | *]
 * 
 * @return SUCCESS (0) or FAILURE (1) 
 */
int	ping(Server *server, int const client_fd, cmd_struct &cmd)
{
	Client		&client		= retrieveClient(server, client_fd);
	std::string	nickname	= client.getNickname();
	std::string	username	= client.getUsername();

	if (cmd.message[0] == ' ')
		cmd.message.erase(0, 1);
	cmd.message.insert(0, ":");
	addToClientBuffer(server, client_fd, RPL_PONG(user_id(nickname, username), cmd.message));

	return (SUCCESS);
}