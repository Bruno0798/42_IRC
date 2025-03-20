#include "../Irc.hpp"
#include "../Server.hpp"
#include "../Client.hpp"
#include <cctype>

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
	lineStream >> newTopic;
	if (channelName.empty())
	{
		send(_clientFd, ERR_NEEDMOREPARAMS(getClient(_clientFd)->getNickname(), "TOPIC").c_str(), ERR_NEEDMOREPARAMS(getClient(_clientFd)->getNickname(), "TOPIC").size(), 0);
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
		send(_clientFd, ERR_NOSUCHCHANNEL(getClient(_clientFd)->getNickname(), channelName).c_str(), ERR_NOSUCHCHANNEL(getClient(_clientFd)->getNickname(), channelName).size(), 0);
		return ;
	}

	if (!LookClientInChannel(channelName))
	{

		send(_clientFd, ERR_NOTONCHANNEL(getClient(_clientFd)->getNickname(), It->first).c_str(), ERR_NOTONCHANNEL(getClient(_clientFd)->getNickname(), It->first).size(), 0);
		return ;
	}

	//std::getline(lineStream >> std::ws, newTopic);
	if (newTopic[0] == ':' && newTopic.size() == 1)
	{
		commandTopic(It->first, newTopic);
		return;;
	}
	else
		newTopic = getFullMsg(newTopic, lineStream);
	commandTopic(It->first, newTopic);
}

void Server::commandTopic(const std::string &channelName, std::string &newTopic)
{
	std::cout << "new topic is: "<< newTopic << std::endl;

	if (newTopic.empty() || !std::isprint(newTopic[0]))
	{
		std::string topicMsg = getChannelTopic(channelName);
		if (topicMsg.empty())
			send(_clientFd, RPL_NOTOPIC(getClient(_clientFd)->getNickname(), channelName).c_str(), RPL_NOTOPIC(getClient(_clientFd)->getNickname(), channelName).size(), 0);
		else
			send(_clientFd, RPL_TOPIC(getClient(_clientFd)->getNickname(), channelName, topicMsg).c_str(), RPL_TOPIC(getClient(_clientFd)->getNickname(), channelName, topicMsg).size(), 0);
		return ;
	}
	std::cout << "new topic is: "<< newTopic << std::endl;
	if (newTopic[0] == ':')
		newTopic = "";

	std::map<std::string, Channel>::iterator It = _channels.find(channelName);
	if (!It->second.isTopicRestricted() || It->second.isOperator(_clientFd))
	{
		std::string topicChange = ":" + getClient(_clientFd)->getNickname() + "!" + getClient(_clientFd)->getUsername()+ "@localhost TOPIC " + It->first + " :" + newTopic + "\r\n";
		broadcastMessageToChannel(topicChange, channelName);
		changeChannelTopic(channelName, newTopic);
	}
	else
		send(_clientFd, ERR_CHANOPRIVSNEEDED(getClient(_clientFd)->getNickname(), It->first).c_str(), ERR_CHANOPRIVSNEEDED(getClient(_clientFd)->getNickname(), It->first).size(), 0);
}
