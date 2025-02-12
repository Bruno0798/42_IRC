/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: diogosan <diogosan@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/05 14:30:28 by diogosan          #+#    #+#             */
/*   Updated: 2025/02/12 15:41:50 by diogosan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"
#include "Server.hpp"
#include <cstddef>
#include <string>
#include <vector>

bool Server::LookClientInChannel(std::string channel)
{
	std::map<std::string, Channel >::const_iterator It = _channels.find(channel);
	if (It != _channels.end())
	{
		const std::map<int, std::vector<std::string> >& clients = It->second.getClients();
		for (std::map<int, std::vector<std::string> >::const_iterator clientIt = clients.begin(); clientIt != clients.end(); ++clientIt)
		{
			if (clientIt->first == _clientFd)
			{
				return true;
			}
		}
	}
	return false;
}
/*
std::string Ircserv::_getChannelTopic(std::string channel)
{
	std::map<std::string, std::string>::const_iterator It = _channelTopics.find(channel);
	
	if (It != _channelTopics.end())
	{
		if (It->first == channel)
			return It->second;
	}
	return NULL;
}


void Ircserv::_changeChannelTopic(std::string &channel, std::string &newTopic)
{
	std::map<std::string, std::string>::iterator It = _channelTopics.find(channel);
	
	if (It != _channelTopics.end())
	{
		if (It->first == channel)
			It->second = newTopic;
	}

}*/