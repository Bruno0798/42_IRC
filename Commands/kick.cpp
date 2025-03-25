#include "../Irc.hpp"
#include "../Client.hpp"
#include "../Server.hpp"
#include "../Channel.hpp"



void Server::handleKick(int client_fd, const std::string& message)
{
    std::istringstream iss(message);
    std::string cmd, channel, targets, reason;
    iss >> cmd >> channel >> targets >> reason;

	reason = getFullMsg(reason, iss, 100);
	std::vector<Client>::iterator kicker = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
	std::map<std::string, Channel>::iterator channel_it = _channels.begin();

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
        send(client_fd, ERR_CHANOPRIVSNEEDED(kicker->getNickname(), channel_it->first).c_str(), ERR_CHANOPRIVSNEEDED(kicker->getNickname(), channel_it->first).length(), 0);
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
            send(client_fd, ERR_NOSUCHNICK(kicker->getNickname(), it->getNickname()).c_str(), ERR_NOSUCHNICK(kicker->getNickname(), it->getNickname()).length(), 0);
            continue;
        }
        if (!channel_it->second.isUserInChannel(target_fd))
        {
            send(client_fd, ERR_USERNOTINCHANNEL(kicker->getNickname(), it->getNickname(), channel_it->first).c_str(), ERR_USERNOTINCHANNEL(kicker->getNickname(), it->getNickname(), channel_it->first).length(), 0);
            continue;
        }

        std::string kick_msg = ":" + kicker->getNickname() + "!" + kicker->getUsername() + 
                               "@localhost KICK " + channel_it->first + " " + it->getNickname() + " " + reason + "\r\n";
        
        broadcastMessageToChannel(kick_msg, channel);
        channel_it->second.revokePermissions(target_fd);
        channel_it->second.removeClient(target_fd);
		if (channel_it->second.getClients().empty())
			_channels.erase(channel_it->first);
    }
}