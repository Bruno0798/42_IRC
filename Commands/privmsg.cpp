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
 *
 * @param server
 * @param client_fd User sending a msg
 * @param cmd_infos Structure w/ prefix, command name and message
 *
 */

void Server::handlePrivmsg(int client_fd, const std::string& message)
{
	std::istringstream iss(message);
	std::string cmd, target, msg;
	iss >> cmd >> target;
	std::getline(iss, msg);

	if (target.empty() || msg.empty())
	{
		std::cerr << "PRIVMSG command requires a target and a message" << std::endl;
		return;
	}

	// Remove leading colon from the message
	if (msg[0] == ':')
	{
		msg = msg.substr(1);
	}

	std::string response = ":";
	std::vector<Client>::iterator client_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
	if (client_it != _clients.end())
	{
		response += client_it->getNickname() + "!" + client_it->getUsername() + "@localhost PRIVMSG " + target + " :" + msg + "\r\n";
	}
	else
	{
		std::cerr << "Client not found for fd: " << client_fd << std::endl;
		return;
	}

	// Check if the target is a channel
	std::map<std::string, Channel>::iterator channel_it = _channels.find(target);
	if (channel_it != _channels.end())
	{
		const Channel& channel = channel_it->second;
		for (std::map<int, std::vector<std::string> >::const_iterator it = channel.getClients().begin(); it != channel.getClients().end(); ++it)
		{
			if (it->first != client_fd)
				send(it->first, response.c_str(), response.size(), 0);
		}
	}
	else
	{
		// Target is a user
		try
		{
			int target_fd = getClientFdByName(target);
			std::vector<Client>::iterator target_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(target_fd));
			if (target_it != _clients.end())
			{
				send(target_it->getFd(), response.c_str(), response.size(), 0);
			}
			else
			{
				std::cerr << "User not found: " << target << std::endl;
			}
		}
		catch (const std::runtime_error& e)
		{
			std::cerr << e.what() << std::endl;
		}
	}
}