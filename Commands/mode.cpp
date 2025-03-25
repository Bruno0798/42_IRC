#include "../Irc.hpp"
#include "../Client.hpp"
#include "../Server.hpp"
#include "../Channel.hpp"



void Server::handleMode(int client_fd, const std::string& message)
{
    std::istringstream iss(message);
    std::string cmd, channel, modes, target;
    iss >> cmd >> channel >> modes;

    std::vector<Client>::iterator client_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
	std::vector<Client>::iterator it = _clients.begin();
	std::map<std::string, Channel>::iterator channel_it = _channels.begin();
	if (channel.empty())
    {
		send(client_fd, ERR_NEEDMOREPARAMS(client_it->getNickname(), "MODE").c_str(), ERR_NEEDMOREPARAMS(client_it->getNickname(), "MODE").length(), 0);
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
        send(client_fd, ERR_NOSUCHCHANNEL(client_it->getNickname(), channel).c_str(), ERR_NOSUCHCHANNEL(client_it->getNickname(), channel).length(), 0);
        return;
    }

    if (modes.empty())
    {
        std::string current_modes = channel_it->second.getModeString();
        std::string response = ":localhost 324 " + client_it->getNickname() + " " + channel + " :" + current_modes + "\r\n";
        send(client_fd, response.c_str(), response.length(), 0);
        return;
    }

    if (!channel_it->second.isOperator(client_fd))
    {
        send(client_fd, ERR_CHANOPRIVSNEEDED(client_it->getNickname(), channel).c_str(), ERR_CHANOPRIVSNEEDED(client_it->getNickname(), channel).length(), 0);
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
					if (adding)
						if(channel_it->second.isInviteOnly())
							continue;
					if (!adding)
						if (!channel_it->second.isInviteOnly())
							continue;
                    channel_it->second.setInviteOnly(adding);
                    applied_modes += (adding ? "+i" : "-i");
                    break;
                case 't':
					if (adding)
						if(channel_it->second.isTopicRestricted())
							continue;
					if (!adding)
						if (!channel_it->second.isTopicRestricted())
							continue;
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
								send(client_fd, ERR_NOSUCHNICK(client_it->getNickname(), target).c_str(), ERR_NOSUCHNICK(client_it->getNickname(), target).length(), 0);
								return;
							}

							if (!channel_it->second.isUserInChannel(target_fd))
							{
								send(client_fd, ERR_USERNOTINCHANNEL(client_it->getNickname(), target, channel).c_str(), ERR_USERNOTINCHANNEL(client_it->getNickname(), target, channel).length(), 0);
								return;
							}

							if (adding)
							{
								if(channel_it->second.isOperator(target_fd))
									continue;;
								channel_it->second.addOperator(target_fd);
							}
							else{
								if(!channel_it->second.isOperator(target_fd))
									continue;
								channel_it->second.removeOperator(target_fd);
							}
							
							applied_modes += (adding ? "+o" : "-o");
							applied_target = it->getNickname();
						}
						catch (const std::runtime_error& e) {
							send(client_fd, ERR_NOSUCHNICK(client_it->getNickname(), target).c_str(), ERR_NOSUCHNICK(client_it->getNickname(), target).length(), 0);
							return;
						}
					}
                    else
                    {
                        send(client_fd, ERR_NEEDMOREPARAMS(client_it->getNickname(), "MODE").c_str(), ERR_NEEDMOREPARAMS(client_it->getNickname(), "MODE").length(), 0);
                        return;
                    }
                    break;
                case 'k':
                    if (adding)
                    {
						if (channel_it->second.isPasswordProtected())
							continue;
                        iss >> extra_param;
                        if (extra_param.empty())
                        {
                            send(client_fd, ERR_NEEDMOREPARAMS(client_it->getNickname(), "MODE").c_str(), ERR_NEEDMOREPARAMS(client_it->getNickname(), "MODE").length(), 0);
                            return;
                        }
                        channel_it->second.setPass(extra_param);
						applied_target = extra_param;
                    }
                    else
                    {
						if (!channel_it->second.isPasswordProtected())
							continue;
                        channel_it->second.removePass();
                    }
                    applied_modes += (adding ? "+k" : "-k");
                    break;
                case 'l':
                    if (adding)
                    {
						if (channel_it->second.hasUserLimit())
							continue;
                        iss >> extra_param;
                        if (extra_param.empty() || !std::isdigit(extra_param[0]))
                        {
                            send(client_fd, ERR_NEEDMOREPARAMS(client_it->getNickname(), "MODE").c_str(), ERR_NEEDMOREPARAMS(client_it->getNickname(), "MODE").length(), 0);
                            return;
                        }
                        int limit = std::atoi(extra_param.c_str());
                        channel_it->second.setUserLimit(limit);
                    }
                    else
                    {
						if (!channel_it->second.hasUserLimit())
							continue;
                        channel_it->second.removeUserLimit();
                    }
                    applied_modes += (adding ? "+l" : "-l");
                    if (adding)
                        applied_target = extra_param;
                    break;
                default:
                    send(client_fd, ERR_UMODEUNKNOWNFLAG(client_it->getNickname()).c_str(), ERR_UMODEUNKNOWNFLAG(client_it->getNickname()).length(), 0);
                    return;
            }
        }
    }

    std::vector<Client>::iterator setter = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
    if (setter != _clients.end())
    {
        std::string mode_msg = ":" + setter->getNickname() + "!" + setter->getUsername() + 
                              "@localhost MODE " + channel_it->first + " " + applied_modes +
                              (applied_target.empty() ? "" : " " + applied_target) + "\r\n";
        broadcastMessageToChannel(mode_msg, channel_it->first);
    }
}
