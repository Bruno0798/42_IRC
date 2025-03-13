#include "../Irc.hpp"
#include "../Server.hpp"
#include "../Client.hpp"

/**
 * @brief Command : TOPIC <channel> [<topic>]
 *
 * 	The TOPIC command is used to change or view the topic of the given channel.
 * 	If <topic> is not given, either RPL_TOPIC or RPL_NOTOPIC is returned
 * 	specifying the current channel topic or lack of one.
 * 	If <topic> is an empty string, the topic for the channel will be cleared.
 *
 * 	Numeric Replies:
 *
 * 	ERR_NEEDMOREPARAMS (461)
 * 	ERR_NOSUCHCHANNEL (403)
 * 	ERR_NOTONCHANNEL (442)
 * 	RPL_NOTOPIC (331)
 * 	RPL_TOPIC (332)
 *
 * 	Examples:
 * 	[CLIENT] TOPIC #test :New topic
 *  [SERVER] ; Setting the topic on "#test" to "New topic".
 *
 * 	[CLIENT] TOPIC #test :
 * 	[SERVER] ; Clearing the topic on "#test"
 *
 * 	[CLIENT] TOPIC #test
 * 	[SERVER] ; Checking the topic for "#test"
 *
 */

void Server::checkCommandTopic(std::istringstream &lineStream)
{
	std::string channelName, newTopic;

	lineStream >> channelName;
	if (channelName.empty())
	{
		std::string errMsg = ":localhost 461 " + channelName + " :Not enough parameters\r\n";
		send(_clientFd, errMsg.c_str(), errMsg.size(), 0);
		return ;
	}

	std::map<std::string, Channel >::iterator It = _channels.begin();

	while (It != _channels.end())
	{
		if (getLower(It->first) == getLower(channelName))
			break;
		++It;
	}
	
	if (channelName.empty()|| channelName[0] != '#' || It == _channels.end())
	{
		std::string errMsg = ":localhost 403 " + channelName + " :No such channel!\r\n";
		send(_clientFd, errMsg.c_str(), errMsg.size(), 0);
		return ;
	}

	if (!LookClientInChannel(channelName))
	{
		std::string errMsg = ":localhost 442 " + It->first + " :User is not in the channel!\r\n";
		send(_clientFd, errMsg.c_str(), errMsg.size(), 0);
		return ;
	}
	std::getline(lineStream >> std::ws, newTopic);
	std::cout << newTopic << std::endl;
	commandTopic(It->first, newTopic);
}

void Server::commandTopic(const std::string &channelName, std::string &newTopic)
{
	if (newTopic.empty() || newTopic[0] == ' ' )
	{
		std::string topic = getChannelTopic(channelName);
		if (topic.empty())
			topic = "No topic is set";
		std::string topicMsg = ":localhost 332 " + getClient(_clientFd)->getNickname() + " " + channelName + " :" + topic + "\r\n";
		send(_clientFd, topicMsg.c_str(), topicMsg.size(), 0);
		return ;
	}

	std::map<std::string, Channel>::iterator It = _channels.find(channelName);
	if (!It->second.isTopicRestricted() || It->second.isOperator(_clientFd))
	{
		if (newTopic[0] == ':')
			newTopic.erase(0,1);
		if (newTopic[0] == ':')
			newTopic.erase(0,1);
		std::string topicChange = ":" + getClient(_clientFd)->getNickname() + "!" + getClient(_clientFd)->getUsername()+ "@localhost TOPIC " + It->first + " :" + newTopic + "\r\n";
		broadcastMessageToChannel(topicChange, channelName);
		changeChannelTopic(channelName, newTopic);
	}
	else
	{
		std::string topicMsg = ":localhost 482 " + getClient(_clientFd)->getNickname() + " " + It->first + " :You're not channel operator\r\n"; 
		send(_clientFd, topicMsg.c_str(), topicMsg.size(), 0);
	}
}
