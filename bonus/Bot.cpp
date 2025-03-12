#include "../Irc.hpp"
#include "../Client.hpp"
#include "../Server.hpp"
#include <vector>

void Server::JoinBot(int client_fd, const std::string& channel_name)
{
	if (channel_name[0] != '#')
	{
		std::string errMsg = ":ircserver 461 " + channel_name + " :Invalid channel name\r\n";
		send(_clientFd, errMsg.c_str(), errMsg.size(), 0);
		return;
	}

	std::map<std::string, Channel>::iterator it = _channels.find(channel_name);
	//std::vector<Client>::iterator client_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));

	// Add client to existing channel
	if (it->second.isOperator(_clientFd))
	{
		it->second.addClient(client_fd);
		std::cout << "Bot Joined existing channel: " << channel_name << std::endl;
	}
	else 
	{
		std::string errorMsg = ":localhost 482 Bot " + channel_name + " :You're not channel operator\r\n"; 
		send(_clientFd, errorMsg.c_str(), errorMsg.size(), 0);
		return;
	}
	std::string response = ":StepBro!StepBro@localhost JOIN " + channel_name + "\r\n";

	makeUserList(channel_name);

}

//pre merge
void Server::checkCommandBot(std::istringstream &lineStream)
{
	std::string channel, order;
	lineStream >> channel;
	lineStream >> order;
	
	if (channel.empty())
	{
		std::string errMsg = ":ircserver 461 " + channel + " :Not enough parameters\r\n";
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
	std::map<std::string, Channel >::iterator It = _channels.find(channelName);
	if (channelName.empty()|| channelName[0] != '#' || It == _channels.end())
	{
		std::string errMsg = ":localhost 403 " + channelName + " :No such channel!\r\n";
		send(_clientFd, errMsg.c_str(), errMsg.size(), 0);
		return ;
	}
	bool inChannel = LookBotInChannel(channelName);
	if (msg == "join" && !inChannel)
	{
		std::cout << "Bot vai dar join "  << std::endl;
		JoinBot(424242, channelName);
		return;
	}
	if (!inChannel)
	{
		std::string errMsg = ":localhost 442 " + channelName + " :BOT is not in the channel!\r\n";
		send(_clientFd, errMsg.c_str(), errMsg.size(), 0);
		return ;
	}
	else if (msg == "part" && It->second.isOperator(_clientFd))
	{
		std::cout << "Bot vai dar part "  << std::endl;
		PartBot(channelName);
		return;
	}
	else if (msg == "talk")
	{
		std::cout << "Bot vai falar "  << std::endl;
		PrivmsgBot(channelName, getMsg());
		return;
	}
}

void Server::PartBot(std::string &channelName)
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
	std::string leaveMsg = ":StepBro!StepBro@localhost PART " + channelName + " " + msg +"\r\n";
	It->second.removeClient(424242);
	makeUserList(channelName);
	
	std::cout << "BOT LEAVE CHANNEL" << channelName <<"\n";
}

void Server::PrivmsgBot(const std::string& channel, const std::string& msg)
{
	std::string response = ":";

	response += ":StepBro!StepBro@localhost PRIVMSG " + channel + " :" + msg + "\r\n";
	std::map<std::string, Channel>::iterator channel_it = _channels.find(channel);
	if (channel_it != _channels.end())
	{
		const Channel& channel = channel_it->second;
		for (std::map<int, std::vector<std::string> >::const_iterator it = channel.getClients().begin(); it != channel.getClients().end(); ++it)
		{
			if (it->first != 424242)
				send(it->first, response.c_str(), response.size(), 0);
			std::cout << "target:" << it->first << "Bot vai falar: "<< response  << std::endl;
		}
	}

}

std::string Server::getMsg()
{
	std::vector<std::string> msgs(7);

	msgs[0] = "Ducking the ruber duck";
	msgs[1] = "yo";
	msgs[2] = "Make lekaaaaaaaaaaaaaaaaas!";
	msgs[3] = "Vai ser com o pe que esta mais a mao";
	msgs[4] = "Na boa bro o IRC e facil";
	msgs[5] = "The cake is a lie";
	msgs[6] = "Oh a waching machine";

	std::srand(std::time(0)); // Seed the random number generator
    int msg = std::rand() % 6;
	
	std::cout << "coord is:"<<msg << std::endl;
	return msgs[msg];
}