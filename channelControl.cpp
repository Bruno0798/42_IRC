/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channelControl.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: diogosan <diogosan@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/05 14:07:24 by diogosan          #+#    #+#             */
/*   Updated: 2025/02/26 11:24:34 by diogosan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Server.hpp"
#include <cstddef>
#include <string>
#include <vector>


void Server::checkCommandPart(std::istringstream &lineStream)
{
	std::string channels, msgs;
	lineStream >> channels;
	lineStream >> msgs;
	std::cout << "aqui bro!!!!!!! "<<msgs << std::endl;
	if (channels.empty())
	{
		std::string errMsg = ":ircserver 461 " + channels + " :Not enough parameters\r\n";
		send(_clientFd, errMsg.c_str(), errMsg.size(), 0);
		return ;
	}
	std::stringstream channelStream(channels), msgsStream(msgs);
	std::string channelName, msg;
	while (std::getline(channelStream, channelName, ','))
	{
		std::getline(msgsStream, msg ,',');
		if (msg[1] == ':')
			msg.erase(0,1);
		if (!channelName.empty())
		{
			if (!msg.empty())
				commandPart(channelName, msg);
			else
				commandPart(channelName, ":Leaving");
		}
	}
}

void Server::commandPart(std::string &channelName, const std::string &msg)
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
	
	std::string leaveMsg = ":" + getClient(_clientFd)->getNickname() + "!" + getClient(_clientFd)->getUsername() + "@localhost PART " + channelName + " " + msg +"\r\n";
	It->second.removeClient(_clientFd);
	send(_clientFd, leaveMsg.c_str(), leaveMsg.size(), 0);
	broadcastMessageToChannel(leaveMsg, channelName);

	if (It->second.getClients().empty())
		_channels.erase(channelName);
		
	std::cout << "LEAVE CHANNEL" << channelName <<"\n";
}


void Server::checkCommandTopic(std::istringstream &lineStream)
{
	std::string channelName;
	std::string newTopic;

	lineStream >> channelName;
	if (channelName.empty() || channelName == "ft_teste")
	{
		std::string errMsg = ":42 461 " + channelName + " :Not enough parameters\r\n";
		send(_clientFd, errMsg.c_str(), errMsg.size(), 0);
		return ;
	}
	
	std::map<std::string, Channel >::const_iterator It = _channels.find(channelName);
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
	std::getline(lineStream >> std::ws, newTopic);
	std::cout << newTopic << std::endl;
	commandTopic(channelName, newTopic);
}

void Server::commandTopic(std::string &channelName, std::string &newTopic)
{
	if (newTopic.empty() || newTopic[0] == ' ' )
	{
		std::string topic = getChannelTopic(channelName);
		if (topic.empty())
			topic = "No topic is set";
		std::string topicMsg = ":42 332 " + getClient(_clientFd)->getNickname() + " " + channelName + " :" + topic + "\r\n";
		send(_clientFd, topicMsg.c_str(), topicMsg.size(), 0);
		return ;
	}
	std::map<std::string, Channel >::const_iterator It = _channels.find(channelName);
	if (!It->second.isTopicRestricted() || It->second.isOperator(_clientFd))
	{
		newTopic.erase(0,1);
		if (newTopic[0] == ':')
			newTopic.erase(0,1);
		std::string topicChange = ":" + getClient(_clientFd)->getNickname() + "!" + getClient(_clientFd)->getUsername()+ "@localhost TOPIC " + channelName + " :" + newTopic + "\r\n";
		broadcastMessageToChannel(topicChange, channelName);
		changeChannelTopic(channelName, newTopic);
	}
	else
	{
		std::string topicMsg = ":42 482 " + getClient(_clientFd)->getNickname() + " " + channelName + " :You're not channel operator\r\n";
		send(_clientFd, topicMsg.c_str(), topicMsg.size(), 0);
	}
}
