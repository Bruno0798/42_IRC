#include "Irc.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include "Channel.hpp"

void Server::checkRegist(int client_fd)
{
	std::vector<Client>::iterator client_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
	if(client_it->getNickname() != "*" && !client_it->getUsername().empty())
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
		if(cmdsCpy == "CAP" || cmds == "WHO")
			;
		else if(!user.isAuth())
		{
			if (cmdsCpy == "PASS") handlePass(client_fd, line);
			else
			{
				std::cout << RED << "ERROR: NOT AUTHENTICATED" << WHITE << std::endl;
				send(client_fd, ERR_NOTAUTHENTICATED(user.getNickname()).c_str(), ERR_NOTAUTHENTICATED(user.getNickname()).size(), 0);
			}
		} else if(!user.isRegistered())
		{
			if (cmdsCpy == "PASS") handlePass(client_fd, line);
			else if (cmdsCpy =="NICK") handleNick(client_fd, line);
			else if (cmdsCpy == "USER") handleUser(client_fd, line);
			else send(client_fd, ERR_NOTREGISTERED(user.getNickname()).c_str(), ERR_NOTREGISTERED(user.getNickname()).size(), 0);
			checkRegist(client_fd);
		} else
		{
			if (cmdsCpy =="PASS") handlePass(client_fd, line);
			else if (cmdsCpy =="NICK") handleNick(client_fd, line);
			else if (cmdsCpy == "USER") handleUser(client_fd, line);
			else if (cmdsCpy == "JOIN") checkCommandJoin(client_fd, cmd);
			else if (cmdsCpy == "PING") handlePing(client_fd, line);
			else if (cmdsCpy == "MODE") handleMode(client_fd, line);
			else if (cmdsCpy == "TOPIC") checkCommandTopic(cmd);
			else if (cmdsCpy == "KICK") handleKick(client_fd, line);
			else if (cmdsCpy == "INVITE") handleInvite(client_fd, line);
			else if (cmdsCpy =="PRIVMSG") handlePrivmsg(client_fd, line);
			else if (cmdsCpy =="PART") checkCommandPart(cmd);
			else
			{
				std::string errMsg = ":localhost 421 " + user.getNickname() + " " + cmds + " :Unknown commands\r\n";
				send(_clientFd, errMsg.c_str(), errMsg.size(), 0);
			}
		}
	}
	user.delete_buffer();
}

int Server::getClientFdByName(const std::string& nickname) {
	for (std::vector<Client>::iterator clientIt = _clients.begin(); clientIt != _clients.end(); ++clientIt) {
		if (clientIt->getNickname() == nickname)
			return clientIt->getFd();
	}
	throw std::runtime_error("Client not found");
}

void Server::handleKick(int client_fd, const std::string& message)
{
    std::istringstream iss(message);
    std::string cmd, channel, targets, reason;
    iss >> cmd >> channel >> targets;

	std::vector<Client>::iterator kicker = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
    std::map<std::string, Channel>::iterator channel_it = _channels.begin();
	if (iss.peek() == ' ')
		iss.get();
	if (iss.peek() == ':')
	{
		iss.get();
		std::getline(iss, reason);
	}
	else
		iss >> reason;

	if (reason.empty() || !std::isprint(reason[1]))
			reason = ":" + kicker->getNickname();
	if (channel.empty() || targets.empty())
    {
		send(client_fd, ERR_NEEDMOREPARAMS(kicker->getNickname(), "KICK").c_str(), ERR_NEEDMOREPARAMS(kicker->getNickname(), "KICK").length(), 0);
		return;
	}

    while (channel_it != _channels.end())
    {
        if (getLower(channel_it->first) == getLower(channel))
            break;
        ++channel_it;
    }
    if (channel_it == _channels.end())
    {
        send(client_fd, ERR_NOSUCHCHANNEL(kicker->getNickname(), channel).c_str(), ERR_NOSUCHCHANNEL(kicker->getNickname(), channel).length(), 0);
        return;
    }
    if (!channel_it->second.isOperator(client_fd))
    {
        send(client_fd, ERR_CHANOPRIVSNEEDED(kicker->getNickname(), channel).c_str(), ERR_CHANOPRIVSNEEDED(kicker->getNickname(), channel).length(), 0);
        return;
    }
    if (kicker == _clients.end())
        return;

    std::istringstream targets_stream(targets);

    while (std::getline(targets_stream, targets, ','))
    {
        int target_fd = -1;
		std::vector<Client>::iterator it = _clients.begin();
        while (it != _clients.end())
        {
            if (getLower(it->getNickname()) == getLower(targets))
            {
                target_fd = it->getFd();
                break;
            }
			++it;
        }

        if (it == _clients.end())
        {
            send(client_fd, ERR_NOSUCHNICK(kicker->getNickname(), targets).c_str(), ERR_NOSUCHNICK(kicker->getNickname(), targets).length(), 0);
            continue;
        }
        if (!channel_it->second.isUserInChannel(target_fd))
        {
            send(client_fd, ERR_USERNOTINCHANNEL(kicker->getNickname(), targets, channel).c_str(), ERR_USERNOTINCHANNEL(kicker->getNickname(), targets, channel).length(), 0);
            continue;
        }

        std::string kick_msg = ":" + kicker->getNickname() + "!" + kicker->getUsername() + 
                               "@localhost KICK " + channel + " " + it->getNickname() + " " + reason + "\r\n";
        
        broadcastMessageToChannel(kick_msg, channel);
        channel_it->second.revokePermissions(target_fd);
        channel_it->second.removeClient(target_fd);
		if (channel_it->second.getClients().empty())
			_channels.erase(channel_it->first);
    }
}

void Server::handleMode(int client_fd, const std::string& message)
{
    std::istringstream iss(message);
    std::string cmd, channel, modes, target;
    iss >> cmd >> channel >> modes;

	std::vector<Client>::iterator it = _clients.begin();
	std::map<std::string, Channel>::iterator channel_it = _channels.begin();
	if (channel.empty())
    {
		send(client_fd, ERR_NEEDMOREPARAMS(it->getNickname(), "MODE").c_str(), ERR_NEEDMOREPARAMS(it->getNickname(), "MODE").length(), 0);
		return;
	}

    while (channel_it != _channels.end())
    {
        if (getLower(channel_it->first) == getLower(channel))
            break;
        ++channel_it;
    }

    if (channel_it == _channels.end())
    {
        send(client_fd, ERR_NOSUCHCHANNEL(it->getNickname(), channel).c_str(), ERR_NOSUCHCHANNEL(it->getNickname(), channel).length(), 0);
        return;
    }

    if (modes.empty())
    {
        std::string current_modes = channel_it->second.getModeString();
        std::string response = ":localhost 324 " + it->getNickname() + " " + channel + " :" + current_modes + "\r\n";
        send(client_fd, response.c_str(), response.length(), 0);
        return;
    }

    if (!channel_it->second.isOperator(client_fd))
    {
        send(client_fd, ERR_CHANOPRIVSNEEDED(it->getNickname(), channel).c_str(), ERR_CHANOPRIVSNEEDED(it->getNickname(), channel).length(), 0);
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
                case 'i':
                    channel_it->second.setInviteOnly(adding);
                    applied_modes += (adding ? "+i" : "-i");
                    break;
                case 't':
                    channel_it->second.setTopicRestricted(adding);
                    applied_modes += (adding ? "+t" : "-t");
                    break;
                case 'o':
					iss >> target;
					if (!target.empty())
					{
						try {
							int target_fd = -1;
							while (it != _clients.end())
							{
								if (getLower(it->getNickname()) == getLower(target))
								{
									target_fd = it->getFd();
									break;
								}
								++it;
							}

							if (target_fd == -1)
							{
								send(client_fd, ERR_NOSUCHNICK(it->getNickname(), target).c_str(), ERR_NOSUCHNICK(it->getNickname(), target).length(), 0);
								return;
							}

							if (!channel_it->second.isUserInChannel(target_fd))
							{
								send(client_fd, ERR_USERNOTINCHANNEL(it->getNickname(), target, channel).c_str(), ERR_USERNOTINCHANNEL(it->getNickname(), target, channel).length(), 0);
								return;
							}

							if (adding)
							{
								if(channel_it->second.isOperator(target_fd))
									return;
								channel_it->second.addOperator(target_fd);
							}
							else{
								if(!channel_it->second.isOperator(target_fd))
									return;
								channel_it->second.removeOperator(target_fd);
							}
							
							applied_modes += (adding ? "+o" : "-o");
							applied_target = target;
						}
						catch (const std::runtime_error& e) {
							send(client_fd, ERR_NOSUCHNICK(it->getNickname(), target).c_str(), ERR_NOSUCHNICK(it->getNickname(), target).length(), 0);
							return;
						}
					}
                    else
                    {
                        send(client_fd, ERR_NEEDMOREPARAMS(it->getNickname(), "MODE").c_str(), ERR_NEEDMOREPARAMS(it->getNickname(), "MODE").length(), 0);
                        return;
                    }
                    break;
                case 'k':
                    if (adding)
                    {
                        iss >> extra_param;
                        if (extra_param.empty())
                        {
                            send(client_fd, ERR_NEEDMOREPARAMS(it->getNickname(), "MODE").c_str(), ERR_NEEDMOREPARAMS(it->getNickname(), "MODE").length(), 0);
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
                case 'l':
                    if (adding)
                    {
                        iss >> extra_param;
                        if (extra_param.empty() || !std::isdigit(extra_param[0]))
                        {
                            send(client_fd, ERR_NEEDMOREPARAMS(it->getNickname(), "MODE").c_str(), ERR_NEEDMOREPARAMS(it->getNickname(), "MODE").length(), 0);
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
                    send(client_fd, ERR_UMODEUNKNOWNFLAG(it->getNickname()).c_str(), ERR_UMODEUNKNOWNFLAG(it->getNickname()).length(), 0);
                    return;
            }
        }
    }

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

	std::vector<Client>::iterator target_it = _clients.begin();
    std::vector<Client>::iterator inviter = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
    std::map<std::string, Channel>::iterator channel_it = _channels.find(channel_name);
    Channel &channel = channel_it->second;

    if (nickname.empty() || channel_name.empty())
    {
        send(client_fd, ERR_NEEDMOREPARAMS(inviter->getNickname(), "INVITE").c_str(), ERR_NEEDMOREPARAMS(inviter->getNickname(), "INVITE").length(), 0);
        return;
    }

    if (channel_it == _channels.end())
    {
        send(client_fd, ERR_NOSUCHCHANNEL(inviter->getNickname(), channel_name).c_str(), ERR_NOSUCHCHANNEL(inviter->getNickname(), channel_name).length(), 0);
        return;
    }

    if (!channel.hasClient(client_fd))
    {
        send(client_fd, ERR_NOTONCHANNEL(inviter->getNickname(), channel_name).c_str(), ERR_NOTONCHANNEL(inviter->getNickname(), channel_name).length(), 0);
        return;
    }

    if (!channel.isOperator(client_fd))
    {
        send(client_fd, ERR_CHANOPRIVSNEEDED(inviter->getNickname(), channel_name).c_str(), ERR_CHANOPRIVSNEEDED(inviter->getNickname(), channel_name).length(), 0);
        return;
    }

    int target_fd = -1;
    while (target_it != _clients.end())
    {
        if (getLower(target_it->getNickname()) == getLower(nickname))
        {
            target_fd = target_it->getFd();
            break;
        }
        ++target_it;
    }
    if (target_it == _clients.end())
    {
        send(client_fd, ERR_NOSUCHNICK(inviter->getNickname(), nickname).c_str(), ERR_NOSUCHNICK(inviter->getNickname(), nickname).length(), 0);
        return;
    }
	
    if (channel.hasClient(target_fd))
    {
        send(client_fd, ERR_USERONCHANNEL(inviter->getNickname(), nickname, channel_name).c_str(), ERR_USERONCHANNEL(inviter->getNickname(), nickname, channel_name).length(), 0);
        return;
    }

    channel.setAllowedClient(target_fd);

    std::string success = ":localhost 341 " + inviter->getNickname() + " " + nickname + " " + channel_name + "\r\n";
    send(client_fd, success.c_str(), success.length(), 0);

    std::string invite_msg = "You have been invited to " + channel_name + " by " + inviter->getNickname() + "\r\n";
    send(target_fd, invite_msg.c_str(), invite_msg.length(), 0);
}
