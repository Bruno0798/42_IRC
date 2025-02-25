#include "../Irc.hpp"
#include "../Server.hpp"
#include "../Client.hpp"

/**
 * @brief The PING command is sent to check the other side of the connection is still
 * 		connected, and/or to check for connection latency.
 *
 * 		When receiving a PING message, clients or servers must reply to it with a
 * 		PONG message with the same <token> value.
 *
 * 		Numeric replies:
 * 		461 - ERR_NEEDMOREPARAMS -  "<client> <command> :Not enough parameters" > DEALT BY IRSSI
 * 		409 - ERR_NOORIGIN - "<client> :No origin specified" (when <token> is empty) > DEALT BY IRSSI
 *
 * 		IRSSI :
 * 		With this reference client, sending a PING to a server sends a CTCP request
 * 		to a nickname or a channel. Using the char '*' pings every user in a channel.
 * 		Syntax : PING [<nick> | <channel> | *]
 *
 * @return SUCCESS (0) or FAILURE (1)
 */

void Server::handlePing(int client_fd, const std::string& message)
{
	std::vector<Client>::iterator client_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
	if (client_it != _clients.end())
	{
		std::string response = ":" + client_it->getNickname() + "!" + client_it->getUsername() + "@localhost ";
		response += "PONG localhost " + message.substr(5) + "\r\n"; // Assuming message is "PING <data>"
		send(client_fd, response.c_str(), response.size(), 0);
	}
}