#include "../Irc.hpp"
#include "../Client.hpp"
#include "../Server.hpp"
#include "../Channel.hpp"


void Server::handleMode(int client_fd, const std::string& message)
{
    std::istringstream iss(message);
    std::string cmd, channel, modes, target, password;
    iss >> cmd >> channel >> modes;

    std::vector<Client>::iterator client_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
	std::vector<Client>::iterator it = _clients.begin();
    std::map<std::string, Channel>::iterator channel_it = _channels.find(channel);

    if (channel.empty())
    {
        send(client_fd, ERR_NEEDMOREPARAMS(client_it->getNickname(), "MODE").c_str(), ERR_NEEDMOREPARAMS(client_it->getNickname(), "MODE").length(), 0);
        return;
    }

    if (channel_it == _channels.end())
    {
        send(client_fd, ERR_NOSUCHCHANNEL(client_it->getNickname(), channel).c_str(), ERR_NOSUCHCHANNEL(client_it->getNickname(), channel).length(), 0);
        return;
    }

    if (!channel_it->second.isOperator(client_fd))
    {
        send(client_fd, ERR_CHANOPRIVSNEEDED(client_it->getNickname(), channel).c_str(), ERR_CHANOPRIVSNEEDED(client_it->getNickname(), channel).length(), 0);
        return;
    }

    if (modes.empty())
    {
        std::string current_modes = channel_it->second.getModeString();
        std::string response = ":localhost 324 " + client_it->getNickname() + " " + channel + " :" + current_modes + "\r\n";
        send(client_fd, response.c_str(), response.length(), 0);
        return;
    }

    std::string applied_modes;
    std::string applied_target;
    std::string error_msg;

    bool adding = true;
    for (size_t i = 0; i < modes.length(); ++i)
    {
        if (modes[i] == '+')
        {
            adding = true;
        }
        else if (modes[i] == '-')
        {
            adding = false;
        }
        else
        {
            switch (modes[i])
            {
                case 'i':
                    if (adding != channel_it->second.isInviteOnly())
                    {
                        channel_it->second.setInviteOnly(adding);
                        applied_modes += (adding ? "+i" : "-i");
                    }
                    break;
                case 't':
                    if (adding != channel_it->second.isTopicRestricted())
                    {
                        channel_it->second.setTopicRestricted(adding);
                        applied_modes += (adding ? "+t" : "-t");
                    }
                    break;
                case 'o':
                    if (!(iss >> target))
                    {
                        error_msg += ERR_NEEDMOREPARAMS(client_it->getNickname(), "MODE");
                        break;
                    }
                    {
                        int target_fd = -1;
                        for (it = _clients.begin(); it != _clients.end(); ++it)
                        {
                            if (getLower(it->getNickname()) == getLower(target))
                            {
                                target_fd = it->getFd();
                                break;
                            }
                        }
                        if (target_fd == -1 || !channel_it->second.isUserInChannel(target_fd))
                        {
                            error_msg += ERR_USERNOTINCHANNEL(client_it->getNickname(), target, channel);
                            break;
                        }
                        if (adding)
                        {
                            if (!channel_it->second.isOperator(target_fd))
                            {
                                channel_it->second.addOperator(target_fd);
                                applied_modes += "+o";
                                applied_target += " " + target;
                            }
                        }
                        else
                        {
                            if (channel_it->second.isOperator(target_fd))
                            {
                                channel_it->second.removeOperator(target_fd);
                                applied_modes += "-o";
                                applied_target += " " + target;
                            }
                        }
                    }
                    break;
                case 'k':
                    if (adding)
                    {
						if (!(iss >> password))
						{
							error_msg += ERR_NEEDMOREPARAMS(client_it->getNickname(), "MODE");
							break;
						}
                        if (channel_it->second.isPasswordProtected() && password != channel_it->second.getPass())
						{
							error_msg += ERR_KEYSET(client_it->getNickname(), channel_it->first);
							break;
						}
						else if (channel_it->second.isPasswordProtected())
							break;
                        channel_it->second.setPass(password);
                        applied_modes += "+k";
                        applied_target += " " + password;
                    }
                    else
                    {
						if (!(iss >> password))
                        {
                            error_msg += ERR_NEEDMOREPARAMS(client_it->getNickname(), "MODE");
                            break;
                        }
                        if (channel_it->second.isPasswordProtected() && password == channel_it->second.getPass())
                        {
                            channel_it->second.removePass();
                            applied_modes += "-k";
                        }
                    }
                    break;
                case 'l':
                    if (adding)
                    {
						if (channel_it->second.hasUserLimit())
							break;
                        if (!(iss >> target) || !std::isdigit(target[0]))
                        {
                            error_msg += ERR_NEEDMOREPARAMS(client_it->getNickname(), "MODE");
                            break;
                        }
                        int limit = std::atoi(target.c_str());
                        channel_it->second.setUserLimit(limit);
                        applied_modes += "+l";
                        applied_target += " " + target;
                    }
                    else
                    {
                        if (channel_it->second.hasUserLimit())
                        {
                            channel_it->second.removeUserLimit();
                            applied_modes += "-l";
                        }
                    }
                    break;
                default:
                    error_msg += ERR_UMODEUNKNOWNFLAG(client_it->getNickname());
                    break;
            }
			adding = true;
        }
    }

    if (!applied_modes.empty())
    {
        std::string mode_msg = ":" + client_it->getNickname() + "!" + client_it->getUsername() +
                               "@localhost MODE " + channel_it->first + " " + applied_modes +
                               (applied_target.empty() ? "" : " " + applied_target) + "\r\n";
        broadcastMessageToChannel(mode_msg, channel_it->first);
    }

    if (!error_msg.empty())
        send(client_fd, error_msg.c_str(), error_msg.length(), 0);
}
