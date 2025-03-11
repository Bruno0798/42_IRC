/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   helpFunctions.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: diogosan <diogosan@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/04 17:42:06 by bde-souz          #+#    #+#             */
/*   Updated: 2025/03/11 16:54:09 by diogosan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"
#include "Server.hpp"
#include <vector>

std::vector<Client>::iterator Server::getClient(int clientFd)
{
	std::vector<Client>::iterator clientIt = _clients.begin();

	while (clientIt != _clients.end())
	{
		if (clientFd == clientIt->getFd())
			return clientIt;
		++clientIt;
	}
	throw std::runtime_error("Client not found");
}


void Server::makeUserList(std::string channel_name)
{
	std::map<std::string, Channel>::const_iterator channelIt = _channels.find(channel_name);
	
	if (channelIt != _channels.end())
	{
		Channel channel = channelIt->second;
		std::map<int, std::vector<std::string> > clients = channel.getClients();
		
		std::string nameList = ":ircserver 353 " + getClient(_clientFd)->getNickname() + " @ " + channel_name + " :";
		

		for (std::map<int, std::vector<std::string> >::iterator It = clients.begin(); It != clients.end(); It++)
		{
			if (It->first == 424242)
			{
				nameList+= "SetpBro ";
				continue;
			}
			if (channel.isOperator(It->first))
				nameList += "@";
			nameList+= getClient(It->first)->getNickname() + " ";
		}
		nameList += "\r\n";

		std::cout << nameList << std::endl;
		broadcastMessageToChannel(nameList, channel_name);
		
		std::string endOfNames = ":ircserver 366 " + getClient(_clientFd)->getNickname() + " " + channel_name + " :End of /NAMES list.\r\n";
		std::cout << endOfNames << std::endl;
		send(_clientFd, endOfNames.c_str(), endOfNames.size(), 0);
	}
	else
		throw std::runtime_error("No server was found!");
}

void Server::broadcastMessageToChannel(const std::string& message, std::string channel)
{
	std::map<std::string, Channel >::const_iterator channelIt = _channels.find(channel);

	if (channelIt != _channels.end())
	{
		Channel channel = channelIt->second;
		std::map<int, std::vector<std::string> > clients = channel.getClients();
		
		for (std::map<int, std::vector<std::string> >::iterator It = clients.begin(); It != clients.end(); It++)
		{
			int clientFd = It->first;
			send(clientFd, message.c_str(), message.size(), 0);
		}
	}
	else
		throw std::runtime_error("No server found in the BroadCast Message");
}


