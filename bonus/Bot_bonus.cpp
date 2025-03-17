#include "../Irc.hpp"
#include "../Client.hpp"
#include "../Server.hpp"
#include <vector>

void Server::JoinBot(int client_fd, const std::string& channel_name)
{
	if (channel_name[0] != '#')
	{
		std::string errMsg = ":localhost 461 " + channel_name + " :Invalid channel name\r\n";
		send(_clientFd, errMsg.c_str(), errMsg.size(), 0);
		return;
	}

	std::map<std::string, Channel>::iterator it = _channels.find(channel_name);
	if (it->second.isOperator(_clientFd))
	{
		it->second.addClient(client_fd);
	}
	else 
	{
		std::string errorMsg = ":localhost 482 Bot " + channel_name + " :You're not channel operator\r\n"; 
		send(_clientFd, errorMsg.c_str(), errorMsg.size(), 0);
		return;
	}
	std::string response = ":Comrade!Comrade@localhost JOIN " + channel_name + "\r\n";

	makeUserList(channel_name);

}

void Server::checkCommandBot(std::istringstream &lineStream)
{
	std::string channel, order;
	lineStream >> channel;
	lineStream >> order;
	
	if (channel.empty())
	{
		std::string errMsg = "BOT usage: /BOT <channelName> ORDER\r\n";
		send(_clientFd, errMsg.c_str(), errMsg.size(), 0);
		errMsg = "ORDERs: join , part , talk\r\n";
		send(_clientFd, errMsg.c_str(), errMsg.size(), 0);
		errMsg = "join: Joins a channel\r\n";
		send(_clientFd, errMsg.c_str(), errMsg.size(), 0);
		errMsg = "part: Parts a channel\r\n";
		send(_clientFd, errMsg.c_str(), errMsg.size(), 0);
		errMsg = "talk: Well...it talks\r\n";
		send(_clientFd, errMsg.c_str(), errMsg.size(), 0);
		return ;
	}

	if (!order.empty())
	{
		if (std::isprint(order[1]))
			commandBot(channel, order);
	}

}

void Server::commandBot(std::string &channelName, const std::string &msg)
{
	std::map<std::string, Channel>::iterator channel = _channels.begin();
	while (channel != _channels.end())
	{
		if (getLower(channel->first) == getLower(channelName))
			break;
		++channel;
	}

	if (channelName.empty()|| channelName[0] != '#' || channel == _channels.end())
	{
		std::string errMsg = ":localhost 403 " + channelName + " :No such channel!\r\n";
		send(_clientFd, errMsg.c_str(), errMsg.size(), 0);
		return ;
	}
	bool inChannel = LookBotInChannel(channelName);
	
	if (msg == "join" && !inChannel)
	{
		//std::cout << "Bot vai dar join "  << std::endl;
		JoinBot(424242, channel->first);
		return;
	}
	if (!inChannel)
	{
		std::string errMsg = ":localhost 442 " + channel->first + " :BOT is not in the channel!\r\n";
		send(_clientFd, errMsg.c_str(), errMsg.size(), 0);
		return ;
	}
	else if (msg == "part" && channel->second.isOperator(_clientFd))
	{
		//std::cout << "Bot vai dar part "  << std::endl;
		PartBot(channel->first);
		return;
	}
	else if (msg == "talk")
	{
		//std::cout << "Bot vai falar "  << std::endl;
		PrivmsgBot(channel->first, getMsg());
		return;
	}
}

void Server::PartBot(const std::string &channelName)
{
	std::map<std::string, Channel >::iterator It = _channels.find(channelName);
	if (channelName.empty()|| channelName[0] != '#' || It == _channels.end())
	{
		std::string errMsg = ":localhost 403 " + channelName + " :No such channel!\r\n";
		send(_clientFd, errMsg.c_str(), errMsg.size(), 0);
		return ;
	}

	if (!LookBotInChannel(channelName))
	{
		std::string errMsg = ":localhost 442 " + channelName + " :BOT is not in the channel!\r\n";
		send(_clientFd, errMsg.c_str(), errMsg.size(), 0);
		return ;
	}
	std::string msg = "Have a nice day :D";
	std::string leaveMsg = ":Comrade!Comrade@localhost PART " + channelName + " " + msg +"\r\n";
	It->second.removeClient(424242);
	makeUserList(channelName);
	
	std::cout << "BOT LEAVE CHANNEL" << channelName <<"\n";
}

void Server::PrivmsgBot(const std::string& channel, const std::string& msg)
{
	std::string response = ":";

	response += ":Comrade!Comrade@localhost PRIVMSG " + channel + " :" + msg + "\r\n";
	std::map<std::string, Channel>::iterator channel_it = _channels.find(channel);
	if (channel_it != _channels.end())
	{
		const Channel& channel = channel_it->second;
		for (std::map<int, std::vector<std::string> >::const_iterator it = channel.getClients().begin(); it != channel.getClients().end(); ++it)
		{
			if (it->first != 424242)
				send(it->first, response.c_str(), response.size(), 0);
			//std::cout << "target:" << it->first << "Bot vai falar: "<< response  << std::endl;
		}
	}

}

std::string Server::getMsg()
{
	std::vector<std::string> msgs(7);

	msgs[0] = "Ducking the ruber duck";
	msgs[1] = "Make lekaaaaaaaaaaaaaaaaas!";
	msgs[2] = "Na boa bro o IRC e facil";
	msgs[3] = "The cake is a lie";
	msgs[4] = "The server is ours comrade!";

	std::srand(std::time(0)); // Seed the random number generator
    int msg = std::rand() % 5;

	return msgs[msg];
}