#include "Irc.hpp"
#include "Channel.hpp"
#include "Server.hpp"
#include "Commands.hpp"

/**
 * @brief The NOTICE command is used to send notices between users, as well as to send notices to channels. 
 * <target> is interpreted the same way as it is for the PRIVMSG command.
 * 
 * The NOTICE message is used similarly to PRIVMSG. 
 * The difference between NOTICE and PRIVMSG is that automatic replies must never be sent in response to a NOTICE message. 
 * This rule also applies to servers – they must not send any error back to the client on receipt of a NOTICE command. 

   @param server
   @param client_fd User sending a message
   @param cmd_infos Structure containing the prefix, command name, and message

   Useful link: https://modern.ircdocs.horse/#notice-message
 * 
 */

static void  broadcastToChannel(Server *server, int const client_fd, std::map<const int, Client>::iterator it_client, std::map<std::string, Channel>::iterator it_channel, std::string message)
{
   // TODO: Check based on channel modes
   // - Check if the user is a member of the channel -> If yes: loop through and send to every user in the channel
   // - If not: check if the channel mode allows sending messages

   std::map<std::string, Client>::iterator member = it_channel->second.getClientList().begin(); // Start of the channel's client list
   while (member != it_channel->second.getClientList().end())
   {
      if (member->second.getClientFd() != client_fd)   // Prevent sending the message back to the sender
         addToClientBuffer(server, member->second.getClientFd(), RPL_PRIVMSG(it_client->second.getNickname(), it_client->second.getUsername(), message));    
      member++;
   }
}

void	notice(Server *server, int const client_fd, cmd_struct cmd_infos)
{  
   std::map<const int, Client>	client_list = server->getClients();
   std::map<std::string, Channel> channel_list = server->getChannels(); 
   std::map<const int, Client>::iterator it_client = client_list.find(client_fd); // Find the client who is sending the message

   // Parsing message 
   size_t      delimiter = cmd_infos.message.rfind(":");
   std::string target = cmd_infos.message.substr(1, (delimiter - 2)); // Ends before the space before the delimiter ':'
   std::string msg_to_send = cmd_infos.message.substr(delimiter);

   // Error handling for message syntax
   if (target.empty())        // No recipient specified
      return ;
   if (msg_to_send.empty())   // No message provided
      return ;

   // Channel case
   if (target[0] == '#')
   {
      std::map<std::string, Channel>::iterator it_channel = channel_list.find(target.substr(1)); // Find channel name by skipping the '#' character
      if (it_channel == channel_list.end())
         return ;
      else
         broadcastToChannel(server, client_fd, it_client, it_channel, cmd_infos.message);
   }
   // User case
   else
   {     
      std::map<const int, Client>::iterator it_target = client_list.begin();
      while (it_target != client_list.end())
      {
         if (it_target->second.getNickname() == target)
             break;
         it_target++;
      }
      if (it_target == client_list.end())
         return ;
      else
         addToClientBuffer(server, it_target->first, RPL_PRIVMSG(it_client->second.getNickname(), it_client->second.getUsername(), cmd_infos.message)); 
   }
}
