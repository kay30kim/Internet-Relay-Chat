#include "Irc.hpp"

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
int	ping(int const client_fd, cmd_struct &cmd)
{
    // Check the string (Does the identifier exist? Is the user registered or online? 
    // Does the channel exist?)
	std::cout << client_fd << " is the client_fd. Message de la cmd : " << YELLOW << cmd.message << RESET << std::endl;
	
    // Send back a PONG with the same token.
	std::string pong_reply = "PONG" + cmd.message + "\r\n";
	send(client_fd, pong_reply.c_str(), pong_reply.size(), 0);
	
	return (SUCCESS);
}