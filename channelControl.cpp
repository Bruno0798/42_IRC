/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channelControl.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migupere <migupere@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/05 14:07:24 by diogosan          #+#    #+#             */
/*   Updated: 2025/03/03 12:23:55 by migupere         ###   ########.fr       */
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
	
	std::cout << "aqui bro!!!!!!!! "<<msgs << std::endl;
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
