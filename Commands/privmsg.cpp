#include "../Irc.hpp"
#include "../Server.hpp"
#include "../Client.hpp"

/**
 * @brief PRIVMSG is used to send private messages between users, as well as to
 * send messages to channels.  <msgtarget> is usually the nickname of
 * the recipient of the message, or a channel name.
 *
 * Parameters: <msgtarget> <text to be sent>
 *
 * The <msgtarget> parameter may also be a host mask (#<mask>) or server
 * mask ($<mask>).  In both cases the server will only send the PRIVMSG
 * to those who have a server or host matching the mask.  The mask MUST
 * have at least 1 (one) "." in it and no wildcards following the last
 * ".".  This requirement exists to prevent people sending messages to
 * "#*" or "$*", which would broadcast to all users.  Wildcards are the
 * '*' and '?'  characters.  This extension to the PRIVMSG command is
 * only available to operators.
 *
 * 	Numeric Replies:
 *		ERR_NOSUCHNICK (401)
 *		ERR_CANNOTSENDTOCHAN (404)
 *		ERR_TOOMANYTARGETS (407)
 *		ERR_NORECIPIENT (411)
 *		ERR_NOTEXTTOSEND (412)
 *		ERR_NOTOPLEVEL (413)
 *		ERR_WILDTOPLEVEL (414)
 *		RPL_AWAY (301)
 */

void Server::handlePrivmsg(int client_fd, const std::string& message)
{
	std::istringstream iss(message);
	std::string cmd, targets, msg;
	iss >> cmd >> targets;
	std::getline(iss, msg);

	std::cout << "Debug: PRIVMSG: CMD: " << cmd << std::endl;
	std::cout << "Debug: PRIVMSG: targets: " << targets << std::endl;
	std::cout << "Debug: PRIVMSG: msg: " << msg << "d" << std::endl;

	std::vector<Client>::iterator client_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
	if (targets.empty())
	{
		std::string response = ":localhost 411 " + client_it->getNickname() + ":No recipient given PRIVMSG\r\n";
		send(client_fd, response.c_str(), response.size(), 0);
		return;
	}
	if (msg.empty())
	{
		std::string response = ":localhost 412 " + client_it->getNickname() + ":No text to send\r\n";
		send(client_fd, response.c_str(), response.size(), 0);
		return;
	}

	// Remove leading colon from the message
	if (msg[0] == ':')
		msg = msg.substr(1);

	std::vector<std::string> targetList;
	std::istringstream targetStream(targets);
	std::string target;
	while (std::getline(targetStream, target, ','))
		targetList.push_back(target);

	std::string response;
	if (client_it != _clients.end())
		response = ":" + client_it->getNickname() + "!" + client_it->getUsername() + "@localhost PRIVMSG ";
	else
		response = ":localhost 401 " + client_it->getNickname() + " :No such nick/channel\r\n";

	for (const std::string& target : targetList)
	{
		std::string fullResponse = response + target + msg + "\r\n";

		// Check if the target is a channel
		std::map<std::string, Channel>::iterator channel_it = _channels.find(target);
		if (channel_it != _channels.end())
		{
			const Channel& channel = channel_it->second;
			std::map<int, std::vector<std::string> >::const_iterator it = channel.getClients().find(client_fd);
			if (it == channel.getClients().end())
			{
				std::string errorResponse = ":localhost 404 " + client_it->getNickname() + " " + target + " :Cannot send to channel\r\n";
				send(client_fd, errorResponse.c_str(), errorResponse.size(), 0);
				continue;
			}

			for (std::map<int, std::vector<std::string> >::const_iterator it = channel.getClients().begin(); it != channel.getClients().end(); ++it)
			{
				if (it->first != client_fd)
					send(it->first, fullResponse.c_str(), fullResponse.size(), 0);
			}
		}
		else
		{
			int target_fd = getClientFdByName(target);
			std::vector<Client>::iterator target_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(target_fd));
			if (target_it != _clients.end())
				send(target_it->getFd(), fullResponse.c_str(), fullResponse.size(), 0);
			else
			{
				response = ":localhost 401 " + client_it->getNickname() + " :No such nick/channel\r\n";
				send(client_fd, response.c_str(), response.size(), 0);
			}
		}
	}
}
