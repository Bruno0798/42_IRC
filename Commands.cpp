#include "Irc.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include "Channel.hpp"

void Server::handleCommand(Client& user, int client_fd)
{
	std::istringstream iss(user.getBuffer());
	std::string cmd;
	iss >> cmd;

	if (cmd == "PING")
		handlePing(client_fd, user.getBuffer());
	else if (cmd == "JOIN")
		handleJoin(client_fd, user.getBuffer());
	else if (cmd == "WHO")
		handleWho(client_fd, user.getBuffer());
	else if (cmd == "PRIVMSG")
		handlePrivmsg(client_fd, user.getBuffer());
	else if (cmd == "NICK")
		handleNick(client_fd, user.getBuffer());
	else if (cmd == "PASS")
		handlePass(client_fd, user.getBuffer());
	else if (cmd == "USER")
		handleUser(client_fd, user.getBuffer());
	else
		std::cerr << "Unknown command: " << cmd << std::endl;
}

void Server::handleUser(int client_fd, const std::string& message)
{
	std::istringstream iss(message);
	std::string cmd, username, hostname, servername, realname;
	iss >> cmd >> username >> hostname >> servername;
	std::getline(iss, realname);

	if (username.empty() || hostname.empty() || servername.empty() || realname.empty())
	{
		std::cerr << "USER command requires username, hostname, servername, and realname" << std::endl;
		return;
	}

	// Remove leading colon from the realname
	if (realname[0] == ':')
	{
		realname = realname.substr(1);
	}

	std::vector<Client>::iterator client_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
	if (client_it != _clients.end())
	{
		client_it->setUserName(username);
		client_it->setRealName(realname);
		std::string response = ":localhost 001 " + client_it->getNickname() + " :User information set\r\n";
		send(client_fd, response.c_str(), response.size(), 0);
	}
	else
	{
		std::cerr << "Client not found for fd: " << client_fd << std::endl;
	}
}

void Server::handlePass(int client_fd, const std::string& message)
{
	std::istringstream iss(message);
	std::string cmd, password;
	iss >> cmd >> password;

	if (password.empty())
	{
		std::cerr << "PASS command requires a password" << std::endl;
		return;
	}

	std::vector<Client>::iterator client_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
	if (client_it != _clients.end())
	{
		client_it->setPassword(password);
		std::string response = ":localhost 001 " + client_it->getNickname() + " :Password set\r\n";
		send(client_fd, response.c_str(), response.size(), 0);
	}
	else
	{
		std::cerr << "Client not found for fd: " << client_fd << std::endl;
	}
}

void Server::handleNick(int client_fd, const std::string& message)
{
	std::istringstream iss(message);
	std::string cmd, nickname;
	iss >> cmd >> nickname;

	if (nickname.empty())
	{
		std::cerr << "NICK command requires a nickname" << std::endl;
		return;
	}

	std::vector<Client>::iterator client_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
	if (client_it != _clients.end())
	{
		client_it->setNickname(nickname);
		std::string response = ":localhost 001 " + nickname + " :Nickname set to " + nickname + "\r\n";
		send(client_fd, response.c_str(), response.size(), 0);
	}
	else
	{
		std::cerr << "Client not found for fd: " << client_fd << std::endl;
	}
}

void Server::handlePrivmsg(int client_fd, const std::string& message)
{
	std::istringstream iss(message);
	std::string cmd, target, msg;
	iss >> cmd >> target;
	std::getline(iss, msg);

	if (target.empty() || msg.empty())
	{
		std::cerr << "PRIVMSG command requires a target and a message" << std::endl;
		return;
	}


	// Remove leading colon from the message
	if (msg[0] == ':')
	{
		msg = msg.substr(1);
	}

	std::string response = ":";
	std::vector<Client>::iterator client_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
	if (client_it != _clients.end())
		response += client_it->getNickname() + "!" + client_it->getUsername() + "@localhost PRIVMSG " + target + " :" + msg + "\r\n";

	// Check if the target is a channel
	std::map<std::string, Channel>::iterator channel_it = _channels.find(target);
	if (channel_it != _channels.end())
	{
		const Channel& channel = channel_it->second;
		for (std::map<int, std::vector<std::string> >::const_iterator it = channel.getClients().begin(); it != channel.getClients().end(); ++it)
		{
			if (it->first != client_fd)
				send(it->first, response.c_str(), response.size(), 0);
		}
	}
	else
	{
		// Target is a user
		std::vector<Client>::iterator target_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
		if (target_it != _clients.end())
			send(target_it->getFd(), response.c_str(), response.size(), 0);
		else
			std::cerr << "User not found: " << target << std::endl;
	}
}


void Server::handlePing(int client_fd, const std::string& message)
{
	std::vector<Client>::iterator client_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
	if (client_it != _clients.end())
	{
		std::string response = ":" + client_it->getNickname() + "!" + client_it->getUsername() + "@localhost ";
		response += "PONG localhost " + message.substr(5) + "\r\n"; // Assuming message is "PING <data>"
		send(client_fd, response.c_str(), response.size(), 0);
	}
}

void Server::handleJoin(int client_fd, const std::string& message)
{
	std::istringstream iss(message);
	std::string cmd, channel_name;
	iss >> cmd >> channel_name;

	if (channel_name.empty())
	{
		std::cerr << "JOIN command requires a channel name" << std::endl;
		return;
	}

	std::map<std::string, Channel>::iterator it = _channels.find(channel_name);
	if (it == _channels.end())
	{
		// Create a new channel if it doesn't exist
		Channel new_channel(channel_name);
		new_channel.addClient(client_fd);
		_channels[channel_name] = new_channel;
		std::cout << "Created and joined new channel: " << channel_name << std::endl;
	}
	else
	{
		// Add client to existing channel
		it->second.addClient(client_fd);
		std::cout << "Joined existing channel: " << channel_name << std::endl;
	}

	std::vector<Client>::iterator client_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
	if (client_it != _clients.end())
	{
		std::string response = ":" + client_it->getNickname() + "!" + client_it->getUsername() + "@localhost JOIN " + channel_name + "\r\n";
		send(client_fd, response.c_str(), response.size(), 0);

		response = ":42 353 " + client_it->getNickname() + " = " + channel_name + " :@" + client_it->getNickname() + "\r\n"; // List the members of the channel
		send(client_fd, response.c_str(), response.size(), 0);
	}
}

void Server::handleWho(int client_fd, const std::string& message)
{
	std::istringstream iss(message);
	std::string cmd, channel_name;
	iss >> cmd >> channel_name;

	if (channel_name.empty())
	{
		std::cerr << "WHO command requires a channel name" << std::endl;
		return;
	}

	std::map<std::string, Channel>::iterator it = _channels.find(channel_name);
	if (it == _channels.end())
	{
		std::cerr << "Channel not found: " << channel_name << std::endl;
		return;
	}

	const Channel& channel = it->second;
	std::string response = ":bsousa-d!bsousa-d@localhost 352 " + channel_name + " :";

	const std::map<int, std::vector<std::string> >& clients = channel.getClients();
	for (std::map<int, std::vector<std::string> >::const_iterator it = clients.begin(); it != clients.end(); ++it)
	{
		int client_fd = it->first;
		std::vector<Client>::iterator client_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
		if (client_it != _clients.end())
		{
			response += client_it->getNickname() + " ";
		}
	}
	response += "\r\n";
	send(client_fd, response.c_str(), response.size(), 0);
}