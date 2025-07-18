#include "../Irc.hpp"
#include "../Client.hpp"
#include "../Server.hpp"
#include <cctype>

/**
 * @brief The JOIN command indicates that the client wants to join the given channel(s),
 * 	each channel using the given key for it. The server receiving the command checks
 * 	whether the client can join the given channel, and processes the request.
 *
 * 	While a client is joined to a channel, they receive all relevant information about
 * 	that channel including the JOIN, PART, KICK, and MODE messages affecting the channel.
 * 	They receive all PRIVMSG and NOTICE messages sent to the channel, and they also
 * 	receive QUIT messages from other clients joined to the same channel (to let them
 * 	know those users have left the channel and the network).
 *
 *  Numeric Replies:
 *
 *  ERR_NEEDMOREPARAMS (461)
 *  ERR_NOSUCHCHANNEL (403)
 *  ERR_TOOMANYCHANNELS (405)
 *  ERR_BADCHANNELKEY (475)
 *  ERR_BANNEDFROMCHAN (474)
 *  ERR_CHANNELISFULL (471)
 *  ERR_INVITEONLYCHAN (473)
 *  ERR_BADCHANMASK (476)
 *  RPL_TOPIC (332)
 *  RPL_TOPICWHOTIME (333)
 *  RPL_NAMREPLY (353)
 *  RPL_ENDOFNAMES (366)
 *
 *  Examples:
 * 	[CLIENT]  JOIN #foobar
 *  [SERVER] ; join channel #foobar.
 *
 * 	[CLIENT]  JOIN #foo,#bar fubar,foobar
 * 	[SERVER]; join channel #foo using key "fubar" and channel #bar using key "foobar".
 */

void Server::checkCommandJoin(int client_fd, std::istringstream &lineStream)
{
	std::string channels, lock;
	lineStream >> channels;
	lineStream >> lock;

	std::vector<Client>::iterator client_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
	if (channels.empty())
	{
		send(_clientFd, ERR_NEEDMOREPARAMS(client_it->getNickname(), "JOIN").c_str(), ERR_NEEDMOREPARAMS(client_it->getNickname(), "JOIN").size(), 0);
		return ;
	}
	std::stringstream channelStream(channels), passStream(lock);
	
	std::string channelName, pass;
	while (std::getline(channelStream, channelName, ','))
	{
		std::getline(passStream, pass ,',');
		if (passStream && pass[1] == ':')
			pass.erase(0,1);
		
		if (!channelName.empty())
		{
			if (!pass.empty() && std::isprint(pass[1]))
				handleJoin(_clientFd, channelName, pass);
			else
				handleJoin(_clientFd, channelName, "");
		}
	}
}

std::string getLower(const std::string& str)
{
	std::string lower = str;
	std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
	return lower;
}

void Server::handleJoin(int client_fd, std::string& channel_name, const std::string& pass)
{
	if (channel_name[0] != '#')
	{
		send(_clientFd, ERR_BADCHANMASK(channel_name).c_str(), ERR_BADCHANMASK(channel_name).size(), 0);
		return;
	}
	bool show = false;
	std::map<std::string, Channel>::iterator it = _channels.begin();
	while (it != _channels.end())
	{
		if (getLower(it->first) == getLower(channel_name))
			break;
		++it;
	}
	if (it == _channels.end() || !it->second.hasClient(_clientFd))
		show = true;
    std::vector<Client>::iterator client_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
	if(channel_name.length() > 51)
	{
		send(_clientFd, ERR_CHANNELLENGTH(client_it->getNickname(), channel_name).c_str(), ERR_CHANNELLENGTH(client_it->getNickname(), channel_name).size(), 0);
		return ;
	}
    if (it == _channels.end())
    {
        Channel new_channel(channel_name);
        new_channel.addClient(client_fd);
        _channels[channel_name] = new_channel;
        _channels[channel_name].setTopic("Great topic bro!");
        _channels[channel_name].addOperator(client_fd);
    } 
    else 
    {
        int join_status = it->second.canJoin(client_fd, pass);
        if (join_status == 471)
		{
            send(client_fd, ERR_CHANNELISFULL(client_it->getNickname(), it->first).c_str(), ERR_CHANNELISFULL(client_it->getNickname(), it->first).size(), 0);
			return;
		}
        else if (join_status == 473)
		{
            send(client_fd, ERR_INVITEONLYCHAN(client_it->getNickname(), it->first).c_str(), ERR_INVITEONLYCHAN(client_it->getNickname(), it->first).size(), 0);
			return;
		}
        else if (join_status == 475)
		{
            send(client_fd, ERR_BADCHANNELKEY(client_it->getNickname(), it->first).c_str(), ERR_BADCHANNELKEY(client_it->getNickname(), it->first).size(), 0);
			return;
		}
        else
            it->second.addClient(client_fd);
    }
	it = _channels.begin();
	while (it != _channels.end())
	{
		if (getLower(it->first) == getLower(channel_name))
			break;
		++it;
	}

	if (show)
	{
		std::string response = ":" + client_it->getNickname() + "!" + client_it->getUsername() + "@localhost JOIN " + it->first + "\r\n";
		send(_clientFd, response.c_str(), response.size(), 0);
		
		if (getChannelTopic(channel_name).empty())
			send(_clientFd, RPL_NOTOPIC(client_it->getNickname(), it->first).c_str(), RPL_NOTOPIC(client_it->getNickname(), it->first).size(), 0);
		else
			send(_clientFd, RPL_TOPIC(client_it->getNickname(), it->first, getChannelTopic(channel_name)).c_str(), RPL_TOPIC(client_it->getNickname(), it->first, getChannelTopic(channel_name)).size(), 0);
		makeUserList(channel_name); //TODO if problems take out of the show
	}
}

	
