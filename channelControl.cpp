/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channelControl.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: diogosan <diogosan@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/05 14:07:24 by diogosan          #+#    #+#             */
/*   Updated: 2025/02/12 17:26:09 by diogosan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Server.hpp"
#include <cstddef>
#include <string>
#include <vector>


void Server::checkCommandPart(std::istringstream &lineStream)
{
	std::string channels;
	lineStream >> channels;

	if (channels.empty())
	{
		std::string errMsg = ":ircserver 461 " + channels + " :Not enough parameters\r\n";
		send(_clientFd, errMsg.c_str(), errMsg.size(), 0);
		return ;
	}

	std::stringstream channelStream(channels);
	std::string channelName;
	int c = 1;
	while (std::getline(channelStream, channelName, ','))
	{
	
		if (!channelName.empty())
			commandPart(channelName);
	}
}

void Server::commandPart(std::string &channelName)
{
	std::map<std::string, Channel >::iterator It = _channels.find(channelName);
	if (channelName.empty()|| channelName[0] != '#' || It == _channels.end())
	{
		std::string errMsg = ":42 403 " + channelName + " :No such channel!\r\n";
		send(_clientFd, errMsg.c_str(), errMsg.size(), 0);
		return ;
	}

	if (!LookClientInChannel(channelName))
	{
		std::string errMsg = ":42 442 " + channelName + " :User is not in the channel!\r\n";
		send(_clientFd, errMsg.c_str(), errMsg.size(), 0);
		return ;
	}
	
	std::string leaveMsg = ":" + getClient(_clientFd)->getNickname() + "!" + getClient(_clientFd)->getUsername() + "@localhost PART " + channelName + "\r\n";
	It->second.removeClient(_clientFd);
	send(_clientFd, leaveMsg.c_str(), leaveMsg.size(), 0);
	broadcastMessageToChannel(leaveMsg, channelName);

	if (It->second.getClients().empty())
		_channels.erase(channelName);
		
	std::cout << "LEAVE CHANNEL" << channelName <<"\n";
}


// void Server::checkCommandTopic(std::istringstream &lineStream)
// {
// 	std::string channelName;
// 	std::string newTopic;

// 	lineStream >> channelName;
// 	lineStream >> newTopic;
// 	if (channelName.empty())
// 	{
// 		std::string errMsg = ":42 461 " + channelName + " :Not enough parameters\r\n";
// 		send(_clientFd, errMsg.c_str(), errMsg.size(), 0);
// 		return ;
// 	}

// 	if (newTopic.empty() || newTopic[0] == ' ' )
// 	{
// 		std::string topic = _getChannelTopic(channelName);
// 		if (topic.empty())
// 			topic = "No topic is set";
// 		std::cout << "the topic on " << channelName << " is " << topic << std::endl;
// 		return;
// 	}
// 	//std::getline(lineStream, newTopic);
// 	commandTopic(channelName, newTopic);
// }

// void Server::commandTopic(std::string &channelName, std::string &newTopic)
// {
// 	std::map<std::string, std::vector<Client>>::const_iterator It = _channels.find(channelName);
// 	if (channelName.empty()|| channelName[0] != '#' || It == _channels.end())
// 	{
// 		std::string errMsg = ":42 403 " + channelName + " :No such channel!\r\n";
// 		send(_clientFd, errMsg.c_str(), errMsg.size(), 0);
// 		return ;
// 	}

// 	std::vector<Client>::const_iterator client = LookClientInChannel(channelName);
// 	if (client == std::vector<Server::Client>::const_iterator())
// 	{
// 		std::string errMsg = ":42 442 " + channelName + " :User is not in the channel!\r\n";
// 		send(_clientFd, errMsg.c_str(), errMsg.size(), 0);
// 		return ;
// 	}
// 	newTopic.erase(0,1);
// 	std::string topicChange = ":" + client->_nickName + "!" + client->_userName + "@localhost TOPIC " + channelName + " :" + newTopic + "\r\n";
// 	broadcastMessageToChannel(topicChange, channelName);
// 	_changeChannelTopic(channelName, newTopic);
// }