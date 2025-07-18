#include "Channel.hpp"
#include "Server.hpp"
#include "Irc.hpp"

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

void Server::makeUserList(std::string channel_name)//isto esta ok
{
	std::map<std::string, Channel>::iterator channelIt = _channels.begin();
	while (channelIt != _channels.end())
	{
		if (getLower(channelIt->first) == getLower(channel_name))
			break;
		++channelIt;
	}
	if (channelIt != _channels.end())
	{
		Channel channel = channelIt->second;
		std::map<int, std::vector<std::string> > clients = channel.getClients();
		
		std::string nameList = ":localhost 353 " + getClient(_clientFd)->getNickname() + " @ " + channel.getName() + " :";
		for (std::map<int, std::vector<std::string> >::iterator It = clients.begin(); It != clients.end(); It++)
		{
			if (channel.isOperator(It->first))
				nameList += "@";
			if (It->first == 424242)
				nameList+= "Comrade ";
			else
				nameList+= getClient(It->first)->getNickname() + " ";
		}
		nameList += "\r\n";
		broadcastMessageToChannel(nameList, channel.getName());
		std::string endOfNames = ":localhost 366 " + getClient(_clientFd)->getNickname() + " " + channel.getName() + " :End of /NAMES list.\r\n";
		send(_clientFd, endOfNames.c_str(), endOfNames.size(), 0);
	}
	else
		throw std::runtime_error("No server was found!");
}

void Server::broadcastMessageToClients(const std::string& message)
{
	for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		int clientFd = it->getFd();
		if (clientFd != 424242)
			send(clientFd, message.c_str(), message.size(), 0);
	}
}

void Server::broadcastMessageToChannel(const std::string& message, std::string channel)
{
	std::map<std::string, Channel>::iterator channelIt = _channels.begin();
	while (channelIt != _channels.end())
	{
		if (getLower(channelIt->first) == getLower(channel))
			break;
		++channelIt;
	}

	if (channelIt != _channels.end())
	{
		Channel channel = channelIt->second;
		std::map<int, std::vector<std::string> > clients = channel.getClients();

		for (std::map<int, std::vector<std::string> >::iterator It = clients.begin(); It != clients.end(); It++)
		{
			int clientFd = It->first;
			if (clientFd != 424242)
				send(clientFd, message.c_str(), message.size(), 0);
		}
	}
	else
		throw std::runtime_error("No server found in the BroadCast Message");
}

std::string getFullMsg(std::string &msg, std::istringstream &lineStream, int size)
{
	if (msg[0] == ':' && msg.size() > 1)
	{
		msg.erase(0,1);
		std::string nextWord;
		while (lineStream >> nextWord)
		{
			msg += " ";
			msg += nextWord;
		}
	}
	return msg.substr(0, size);
}
