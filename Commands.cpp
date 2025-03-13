#include "Irc.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include "Channel.hpp"

void Server::checkRegist(int client_fd)
{
	std::vector<Client>::iterator client_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
	if(!client_it->getNickname().empty() && !client_it->getUsername().empty())
    {
		client_it->setRegistered(true);
		welcome_messages(client_fd);
	}
}

void Server::handleCommand(Client& user, int client_fd)
{
	std::istringstream iss(user.getBuffer());
	std::string line, cmds;
	_clientFd = client_fd;

	while(getline(iss, line))
	{
		std::istringstream cmd(line);
		cmd >> cmds;
		std::string cmdsCpy = cmds;
		std::transform(cmdsCpy.begin(), cmdsCpy.end(), cmdsCpy.begin(), ::toupper);
		std::cout << GREEN << "RECEIVED:" << line << WHITE << std::endl;
		if(cmds == "CAP" || cmds == "WHO")
			;
		else if(!user.isAuth())
		{
			if (cmdsCpy == "PASS") handlePass(client_fd, line);
			else
			{
				std::cout << RED << "ERROR: NOT AUTHENTICATED" << WHITE << std::endl;
				std::string response = ":localhost 451 :You have not authenticated\r\n";
				send(client_fd, response.c_str(), response.size(), 0);
			}
		} else if(!user.isRegistered())
		{
			if (cmdsCpy == "PASS") handlePass(client_fd, line);
			else if (cmdsCpy =="NICK") handleNick(client_fd, line);
			else if (cmdsCpy == "USER") handleUser(client_fd, line);
			checkRegist(client_fd);
		} else
		{
			if (cmdsCpy =="PASS") handlePass(client_fd, line);
			else if (cmdsCpy =="NICK") handleNick(client_fd, line);
			else if (cmdsCpy == "USER") handleUser(client_fd, line);
			else if (cmdsCpy == "JOIN") checkCommandJoin(cmd);
			else if (cmdsCpy == "PING") handlePing(client_fd, line);
			else if (cmdsCpy == "MODE") handleMode(client_fd, line);
			else if (cmdsCpy == "TOPIC") checkCommandTopic(cmd);
			else if (cmdsCpy == "KICK") handleKick(client_fd, line);
			else if (cmdsCpy == "INVITE") handleInvite(client_fd, line);
			else if (cmdsCpy =="PRIVMSG") handlePrivmsg(client_fd, line);
			else if (cmdsCpy =="PART") checkCommandPart(cmd);
		}
	}
	user.delete_buffer();
}

int Server::getClientFdByName(const std::string& nickname) {
	for (std::vector<Client>::iterator clientIt = _clients.begin(); clientIt != _clients.end(); ++clientIt) {
		if (clientIt->getNickname() == nickname) {
			return clientIt->getFd();
		}
	}
	throw std::runtime_error("Client not found");
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
    std::string cmd, channel, targets, reason;
    iss >> cmd >> channel >> targets;
    std::getline(iss, reason);

    if (channel.empty() || targets.empty())
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
if (!channel_it->second.isOperator(client_fd))
{
    std::string error = ":localhost 482 " + channel + " :You're not channel operator\r\n";
    send(client_fd, error.c_str(), error.length(), 0);
    return;
}
std::vector<Client>::iterator kicker = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
if (kicker == _clients.end())
    return;
std::istringstream targets_stream(targets);
    std::string target;

    while (std::getline(targets_stream, target, ','))
    {
    try {
        int target_fd = getClientFdByName(target);
        if (!reason.empty() && reason[0] == ' ')
                reason = reason.substr(1);
            if (reason.empty())
                reason = ":" + kicker->getNickname();
            else if (reason[0] != ':')
                reason = ":" + reason;

        std::string kick_msg = ":" + kicker->getNickname() + "!" + kicker->getUsername() + 
                             "@localhost KICK " + channel + " " + target + " " + reason + "\r\n";
        
        broadcastMessageToChannel(kick_msg, channel);
		channel_it->second.revokePermissions(target_fd);
        channel_it->second.removeClient(target_fd);
    }
    catch (const std::runtime_error& e) {
        std::string error = ":localhost 441 " + target + " " + channel + " :They aren't on that channel\r\n";
        send(client_fd, error.c_str(), error.length(), 0);
    }
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
						applied_target = extra_param;
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

void Server::handleInvite(int client_fd, const std::string& message)
{
    std::istringstream iss(message);
    std::string cmd, nickname, channel_name;
    iss >> cmd >> nickname >> channel_name;

    if (nickname.empty() || channel_name.empty())
    {
        std::string error = ":localhost 461 INVITE :Not enough parameters\r\n";
        send(client_fd, error.c_str(), error.length(), 0);
        return;
    }

    std::map<std::string, Channel>::iterator channel_it = _channels.find(channel_name);
    if (channel_it == _channels.end())
    {
        std::string error = ":localhost 403 " + channel_name + " :No such channel\r\n";
        send(client_fd, error.c_str(), error.length(), 0);
        return;
    }

    Channel &channel = channel_it->second;

    if (!channel.hasClient(client_fd))
    {
        std::string error = ":localhost 442 " + channel_name + " :You're not on that channel\r\n";
        send(client_fd, error.c_str(), error.length(), 0);
        return;
    }

    if (!channel.isOperator(client_fd))
    {
        std::string error = ":localhost 482 " + channel_name + " :You're not channel operator\r\n";
        send(client_fd, error.c_str(), error.length(), 0);
        return;
    }

    int target_fd;
    try
    {
        target_fd = getClientFdByName(nickname);
    }
    catch (const std::runtime_error &e)
    {
        std::string error = ":localhost 401 " + nickname + " :No such nick\r\n";
        send(client_fd, error.c_str(), error.length(), 0);
        return;
    }

    std::vector<Client>::iterator inviter = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
    if (inviter == _clients.end())
    {
        std::string error = ":localhost 401 " + nickname + " :No such nick\r\n";
        send(client_fd, error.c_str(), error.length(), 0);
        return;
    }

    std::string inviter_nickname = inviter->getNickname();

	if (channel.hasClient(target_fd))
	{
		std::string error = ":localhost 443 " + inviter_nickname + " " + nickname + " " + channel_name + " :is already on channel\r\n";
		send(client_fd, error.c_str(), error.length(), 0);
		return;
	}

    channel.setAllowedClient(target_fd);

    std::string success = ":localhost 341 " + inviter_nickname + " " + nickname + " " + channel_name + "\r\n";
    send(client_fd, success.c_str(), success.length(), 0);
    
    std::string invite_msg = "You have been invited to " + channel_name + " by " + inviter_nickname + "\r\n";
    send(target_fd, invite_msg.c_str(), invite_msg.length(), 0);
}
