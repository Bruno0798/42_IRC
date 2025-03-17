/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   part.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: diogosan <diogosan@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/05 14:07:24 by diogosan          #+#    #+#             */
/*   Updated: 2025/03/17 20:39:59 by diogosan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Server.hpp"
#include <cctype>
#include <cstddef>
#include <string>
#include <vector>


void Server::checkCommandPart(std::istringstream &lineStream)
{
	std::string channels, msg, next;
	lineStream >> channels;
	lineStream >> msg;


	if (channels.empty())
	{
		std::string errMsg = ":ircserver 461 " + channels + " :Not enough parameters\r\n";
		send(_clientFd, errMsg.c_str(), errMsg.size(), 0);
		return ;
	}
	
	msg = getFullMsg(msg, lineStream);

	std::stringstream channelStream(channels);
	std::string channelName;
	while (std::getline(channelStream, channelName, ','))
	{

		if (!channelName.empty())
		{
			if (!msg.empty() && std::isprint(msg[1]))
				commandPart(channelName, msg);
			else
				commandPart(channelName, ":Leaving");
		}
	}
}

void Server::commandPart(std::string &channelName, const std::string &msg)
{
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

	if (!LookClientInChannel(It->first))
	{
		std::string errMsg = ":localhost 442 " + It->first + " :User is not in the channel!\r\n";
		send(_clientFd, errMsg.c_str(), errMsg.size(), 0);
		return ;
	}
	
	std::string leaveMsg = ":" + getClient(_clientFd)->getNickname() + "!" + getClient(_clientFd)->getUsername() + "@localhost PART " + It->first + " " + msg +"\r\n";
	It->second.removeClient(_clientFd);
	send(_clientFd, leaveMsg.c_str(), leaveMsg.size(), 0);
	broadcastMessageToChannel(leaveMsg, It->first);

	if (It->second.getClients().empty())
		_channels.erase(It->first);
	
	
	//std::cout << "LEAVE CHANNEL" << It->first <<"\n";
}

