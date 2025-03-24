#include "../Irc.hpp"
#include "../Client.hpp"
#include "../Server.hpp"
#include "../Channel.hpp"

void Server::handleInvite(int client_fd, const std::string& message)
{
    std::istringstream iss(message);
    std::string cmd, nickname, channel_name;
    iss >> cmd >> nickname >> channel_name;

	std::vector<Client>::iterator target_it = _clients.begin();
    std::vector<Client>::iterator inviter = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));

	std::map<std::string, Channel>::iterator channel_it = _channels.begin();
	while (channel_it != _channels.end())
	{
		if (getLower(channel_it->first) == getLower(channel_name))
			break;
		++channel_it;
	}
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

    std::string invite_msg =  ":" + inviter->getNickname() +"!"+ inviter->getUsername() + "@localhost INVITE " + target_it->getNickname() + " " + channel_name  + "\r\n";
    send(target_fd, invite_msg.c_str(), invite_msg.length(), 0);
}