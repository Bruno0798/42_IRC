#include "Irc.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include "Channel.hpp"

void Server::handleCommand(Client& user, int client_fd)
{
	std::istringstream iss(user.getBuffer());
	std::string cmd;
	iss >> cmd;

	_clientFd = client_fd;


	if (user.isAuth())
	{

		if (cmd == "PING")
			handlePing(client_fd, user.getBuffer());
		else if (cmd == "JOIN")
			handleJoin(client_fd, user.getBuffer());
		else if (cmd =="PART")
			checkCommandPart(iss);
    else if (cmd == "TOPIC")
		  checkCommandTopic(iss);
		//else if (cmd == "WHO")
		//	handleWho(client_fd, command);
		else if (cmd =="PRIVMSG")
			handlePrivmsg(client_fd, user.getBuffer());
		else if (cmd =="NICK")
			handleNick(client_fd, user.getBuffer());
		else if (cmd =="PASS")
			handlePass(client_fd, user.getBuffer());
		else if (cmd =="USER")
			handleUser(client_fd, user.getBuffer());
    else if (cmd == "MODE")
      handleMode(client_fd, user.getBuffer());
    else if (cmd == "KICK")
      handleKick(client_fd, user.getBuffer());
	// else if (cmd == "INVITE")
	// 	handleInvite(client_fd, user.getBuffer());
		else
			std::cerr << "Unknown command: " << cmd << std::endl;
}

void Server::handleUser(int client_fd, const std::string& message)
{
	std::istringstream iss(message);
	std::string cmd, username, hostname, servername, realname;
	iss >> cmd >> username >> hostname >> servername;
	std::getline(iss, realname);

	if (username.empty() || hostname.empty() || servername.empty() || realname.empty())
	{
		std::cerr << "USER command requires username, hostname, servername, and realname" << std::endl;
		return;
	}

	// Remove leading colon from the realname
	if (realname[0] == ':')
	{
		realname = realname.substr(1);
	}

	std::vector<Client>::iterator client_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
	if (client_it != _clients.end())
	{
		client_it->setUserName(username);
		client_it->setRealName(realname);
		std::string response = ":localhost 001 " + client_it->getNickname() + " :User information set\r\n";
		send(client_fd, response.c_str(), response.size(), 0);
	}
	else
	{
		std::cerr << "Client not found for fd: " << client_fd << std::endl;
	}
}

void Server::handlePass(int client_fd, const std::string& message)
{
	std::istringstream iss(message);
	std::string cmd, password;
	iss >> cmd >> password;

	if (password.empty())
	{
		std::cerr << "PASS command requires a password" << std::endl;
		return;
	}

	std::vector<Client>::iterator client_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
	if (client_it != _clients.end())
	{
		client_it->setPassword(password);
		std::string response = ":localhost 001 " + client_it->getNickname() + " :Password set\r\n";
		send(client_fd, response.c_str(), response.size(), 0);
	}
	else
	{
		std::cerr << "Client not found for fd: " << client_fd << std::endl;
	}
}

void Server::handleNick(int client_fd, const std::string& message)
{
	std::istringstream iss(message);
	std::string cmd, nickname;
	iss >> cmd >> nickname;

	if (nickname.empty())
	{
		std::cerr << "NICK command requires a nickname" << std::endl;
		return;
	}

	std::vector<Client>::iterator client_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
	if (client_it != _clients.end())
	{
		client_it->setNickname(nickname);
		std::string response = ":localhost 001 " + nickname + " :Nickname set to " + nickname + "\r\n";
		send(client_fd, response.c_str(), response.size(), 0);
	}
	else
	{
		std::cerr << "Client not found for fd: " << client_fd << std::endl;
	}
}

int Server::getClientFdByName(const std::string& nickname) {
	for (std::vector<Client>::iterator clientIt = _clients.begin(); clientIt != _clients.end(); ++clientIt) {
		if (clientIt->getNickname() == nickname) {
			return clientIt->getFd();
		}
	}
	throw std::runtime_error("Client not found");
}

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
		response += client_it->getNickname() + "!" + client_it->getUsername() + "@localhost PRIVMSG " + target + " :" + msg + "\r\n";

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
		std::vector<Client>::iterator target_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(getClientFdByName(target)));
		if (target_it != _clients.end())
			send(target_it->getFd(), response.c_str(), response.size(), 0);
		else
			std::cerr << "User not found: " << target << std::endl;
	}
}

void Server::handleWho(int client_fd, const std::string& message)
{
	std::istringstream iss(message);
	std::string cmd, channel_name;
	iss >> cmd >> channel_name;

	if (channel_name.empty())
	{
		std::cerr << "WHO command requires a channel name" << std::endl;
		return;
	}

	std::map<std::string, Channel>::iterator it = _channels.find(channel_name);
	if (it == _channels.end())
	{
		std::cerr << "Channel not found: " << channel_name << std::endl;
		return;
	}

	const Channel& channel = it->second;
	std::string response = ":bsousa-d!bsousa-d@localhost 352 " + channel_name + " :";

	const std::map<int, std::vector<std::string> >& clients = channel.getClients();
	for (std::map<int, std::vector<std::string> >::const_iterator it = clients.begin(); it != clients.end(); ++it)
	{
		int client_fd = it->first;
		std::vector<Client>::iterator client_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
		if (client_it != _clients.end())
		{
			response += client_it->getNickname() + " ";
		}
	}
	response += "\r\n";
	send(client_fd, response.c_str(), response.size(), 0);
}

void Server::handleKick(int client_fd, const std::string& message)
{
    std::istringstream iss(message);
    std::string cmd, channel, target, reason;
    iss >> cmd >> channel >> target;
    std::getline(iss, reason);

    if (channel.empty() || target.empty())
    {
        std::string error = ":localhost 461 KICK :Not enough parameters\r\n";
        send(client_fd, error.c_str(), error.length(), 0);
        return;
    }
    // Find the channel
    std::map<std::string, Channel>::iterator channel_it = _channels.find(channel);
    if (channel_it == _channels.end())
    {
        std::string error = ":localhost 403 " + channel + " :No such channel\r\n";
        send(client_fd, error.c_str(), error.length(), 0);
        return;
    }

    // Find the kicker (operator)
   std::vector<Client>::iterator kicker = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
    if (kicker == _clients.end())
        return;

    // Check if kicker is an operator
    if (!channel_it->second.isOperator(client_fd))
    {
        std::string error = ":localhost 482 " + channel + " :You're not channel operator\r\n";
        send(client_fd, error.c_str(), error.length(), 0);
        return;
    }

    // Find target user's fd
    try {
        int target_fd = getClientFdByName(target);
        
        channel_it->second.removeClient(target_fd);

        // Format reason
        if (reason.empty())
            reason = " :No reason given";
        else if (reason[0] == ':')
            reason = reason.substr(1);

        // Send kick message to channel
        std::string kick_msg = ":" + kicker->getNickname() + "!" + kicker->getUsername() + 
                             "@localhost KICK " + channel + " " + target + reason + "\r\n";
        
        broadcastMessageToChannel(kick_msg, channel);
    }
    catch (const std::runtime_error& e) {
        std::string error = ":localhost 441 " + target + " " + channel + " :They aren't on that channel\r\n";
        send(client_fd, error.c_str(), error.length(), 0);
    }
}

void Server::handleMode(int client_fd, const std::string& message)
{
    std::istringstream iss(message);
    std::string cmd, channel, modes, target;
    iss >> cmd >> channel >> modes;

    if (channel.empty())
    {
        std::string error = ":localhost 461 MODE :Not enough parameters\r\n";
        send(client_fd, error.c_str(), error.length(), 0);
        return;
    }

    // Find the channel
    std::map<std::string, Channel>::iterator channel_it = _channels.find(channel);
    if (channel_it == _channels.end())
    {
        std::string error = ":localhost 403 " + channel + " :No such channel\r\n";
        send(client_fd, error.c_str(), error.length(), 0);
        return;
    }

    // If no modes specified, return current channel modes
    if (modes.empty())
    {
        std::string current_modes = channel_it->second.getModeString();
        std::string response = ":localhost 324 " + channel + " " + current_modes + "\r\n";
        send(client_fd, response.c_str(), response.length(), 0);
        return;
    }

    if (!channel_it->second.isOperator(client_fd))
    {
        std::string error = ":localhost 482 " + channel + " :You're not channel operator\r\n";
        send(client_fd, error.c_str(), error.length(), 0);
        return;
    }

    bool adding = true;
    std::string applied_modes;
    std::string applied_target;
    std::string extra_param;

    for (size_t i = 0; i < modes.length(); ++i)
    {
        if (modes[i] == '+')
            adding = true;
        else if (modes[i] == '-')
            adding = false;
        else
        {
            switch (modes[i])
            {
                case 'i': // Invite-only
                    channel_it->second.setInviteOnly(adding);
                    applied_modes += (adding ? "+i" : "-i");
                    break;
                case 't': // Topic restriction
                    channel_it->second.setTopicRestricted(adding);
                    applied_modes += (adding ? "+t" : "-t");
                    break;
                case 'o': // Operator status
                    iss >> target;
                    if (!target.empty())
                    {
                        try {
                            int target_fd = getClientFdByName(target);
                            if (adding)
                                channel_it->second.addOperator(target_fd);
                            else
                                channel_it->second.removeOperator(target_fd);
                            applied_modes += (adding ? "+o" : "-o");
                            applied_target = target;
                        }
                        catch (const std::runtime_error& e) {
                            std::string error = ":localhost 401 " + target + " :No such nick\r\n";
                            send(client_fd, error.c_str(), error.length(), 0);
                            return;
                        }
                    }
                    else
                    {
                        std::string error = ":localhost 461 MODE :Missing target for +o/-o\r\n";
                        send(client_fd, error.c_str(), error.length(), 0);
                        return;
                    }
                    break;
                case 'k': // Channel key (password)
                    if (adding)
                    {
                        iss >> extra_param;
                        if (extra_param.empty())
                        {
                            std::string error = ":localhost 461 MODE :Missing parameter for +k\r\n";
                            send(client_fd, error.c_str(), error.length(), 0);
                            return;
                        }
                        channel_it->second.setPass(extra_param);
                    }
                    else
                    {
                        channel_it->second.removePass();
                    }
                    applied_modes += (adding ? "+k" : "-k");
                    break;
                case 'l': // User limit
                    if (adding)
                    {
                        iss >> extra_param;
                        if (extra_param.empty() || !std::isdigit(extra_param[0]))
                        {
                            std::string error = ":localhost 461 MODE :Invalid parameter for +l\r\n";
                            send(client_fd, error.c_str(), error.length(), 0);
                            return;
                        }
                        int limit = std::atoi(extra_param.c_str());
                        channel_it->second.setUserLimit(limit);
                    }
                    else
                    {
                        channel_it->second.removeUserLimit();
                    }
                    applied_modes += (adding ? "+l" : "-l");
                    if (adding)
                        applied_target = extra_param;
                    break;
                default:
                    std::string error = ":localhost 501 " + channel + " :Unknown mode character\r\n";
                    send(client_fd, error.c_str(), error.length(), 0);
                    return;
            }
        }
    }

    // Notify channel members about mode change
    std::vector<Client>::iterator setter = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
    if (setter != _clients.end())
    {
        std::string mode_msg = ":" + setter->getNickname() + "!" + setter->getUsername() + 
                              "@localhost MODE " + channel + " " + applied_modes +
                              (applied_target.empty() ? "" : " " + applied_target) + "\r\n";
        broadcastMessageToChannel(mode_msg, channel);
    }
}

