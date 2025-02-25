#include "../Irc.hpp"
#include "../Client.hpp"
#include "../Server.hpp"

/**
 * @brief The JOIN command indicates that the client wants to join the given channel(s),
 * 	each channel using the given key for it. The server receiving the command checks
 * 	whether or not the client can join the given channel, and processes the request.
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

void Server::handleJoin(int client_fd, const std::string& message)
{
	std::istringstream iss(message);
	std::string cmd, channel_name;
	iss >> cmd >> channel_name;

	if (channel_name.empty())
	{
		std::cerr << "JOIN command requires a channel name" << std::endl;
		return;
	}

	std::map<std::string, Channel>::iterator it = _channels.find(channel_name);
	if (it == _channels.end())
	{
		// Create a new channel if it doesn't exist
		Channel new_channel(channel_name);
		new_channel.addClient(client_fd);
		_channels[channel_name] = new_channel;
		_channels[channel_name].setTopic("Great topic bro!" );
		_channels[channel_name].addOperator(_clientFd);
		std::cout << "Created and joined new channel: " << channel_name << std::endl;
	}
	else
	{
		// Add client to existing channel
		it->second.addClient(client_fd);
		std::cout << "Joined existing channel: " << channel_name << std::endl;
	}

	std::vector<Client>::iterator client_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
	if (client_it != _clients.end())
	{
		std::string response = ":" + client_it->getNickname() + "!" + client_it->getUsername() + "@localhost JOIN " + channel_name + "\r\n";

		std::cout << "fd: "<< _clientFd << " | " << response << std::endl;
		send(_clientFd, response.c_str(), response.size(), 0);

		std::string msgTopic = ":42 332 " + client_it->getNickname() + " " + channel_name + " :" + getChannelTopic(channel_name) + "\r\n";
		send(_clientFd, msgTopic.c_str(), msgTopic.size(), 0);

		makeUserList(channel_name);
	}

}

