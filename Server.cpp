#include "Server.hpp"
#include "Irc.hpp"
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>
#include <vector>

Server::Server() {}

Server::Server(int port, std::string& password)
		: _port(port), _password(password) {}

Server::~Server() {}

bool Server::fillServerInfo(char *port)
{
	memset(&_address, 0, sizeof(_address));
	_address.ai_family = AF_UNSPEC;
	_address.ai_socktype = SOCK_STREAM;
	_address.ai_flags = AI_PASSIVE;

	int rv = getaddrinfo(NULL, port, &_address, &_servinfo);
	if (rv != 0)
	{
		std::cerr << "getaddrinfo error: " << gai_strerror(rv) << std::endl;
		return false;
	}
	return true;
}

bool Server::initServer()
{
	_fd = socket(_servinfo->ai_family, _servinfo->ai_socktype, _servinfo->ai_protocol);
	if (_fd == -1)
	{
		perror("socket");
		return false;
	}

	int yes = 1;
	if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
	{
		perror("setsockopt");
		return false;
	}

	if (bind(_fd, _servinfo->ai_addr, _servinfo->ai_addrlen) == -1)
	{
		close(_fd);
		perror("bind");
		return false;
	}

	freeaddrinfo(_servinfo);

	if (listen(_fd, 10) == -1)
	{
		perror("listen");
		return false;
	}

	return true;
}

void Server::handleCommand(const std::string& command, int client_fd)
{
	std::istringstream iss(command);
	std::string cmd;
	iss >> cmd;

	if (cmd == "PING")
	{
		handlePing(client_fd, command);
	}
	else if (cmd == "JOIN")
	{
		handleJoin(client_fd, command);
	}
	else if (cmd == "WHO")
	{
		handleWho(client_fd, command);
	}
	else if (cmd == "PRIVMSG")
	{
		handlePrivmsg(client_fd, command);
	}
	else
	{
		std::cerr << "Unknown command: " << cmd << std::endl;
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
	{
		response += client_it->getNickname() + "!" + client_it->getUsername() + "@localhost PRIVMSG " + target + " :" + msg + "\r\n";
	}

	// Check if the target is a channel
	std::map<std::string, Channel>::iterator channel_it = _channels.find(target);
	if (channel_it != _channels.end())
	{
		const Channel& channel = channel_it->second;
		for (std::vector<int>::const_iterator it = channel.getClients().begin(); it != channel.getClients().end(); ++it)
		{
			if (*it != client_fd)
			{
				send(*it, response.c_str(), response.size(), 0);
			}
		}
	}
	else
	{
		// Target is a user
		std::vector<Client>::iterator target_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
		if (target_it != _clients.end())
		{
			send(target_it->getFd(), response.c_str(), response.size(), 0);
		}
		else
		{
			std::cerr << "User not found: " << target << std::endl;
		}
	}
}

void Server::handlePing(int client_fd, const std::string& message)
{

	std::string response = ":bsousa-d!bsousa-d@localhost ";
	response += "PONG localhost " + message.substr(5) + "\r\n"; // Assuming message is "PING <data>"
	send(client_fd, response.c_str(), response.size(), 0);
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
	//todo: change to source
	std::string response = ":bsousa-d!bsousa-d@localhost ";
	response += "JOIN " + channel_name + "\r\n";
	send(client_fd, response.c_str(), response.size(), 0);
	response = ":42 353 bsousa-d = #teste :@bsousa-d ola\r\n";
	send(client_fd, response.c_str(), response.size(), 0);

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

	for (int client_fd : channel.getClients())
	{
		std::vector<Client>::iterator client_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
		if (client_it != _clients.end())
		{
			response += client_it->getNickname() + " ";
		}
	}

	response += "\r\n";
	send(client_fd, response.c_str(), response.size(), 0);
}

void Server::runServer()
{
	std::vector<struct pollfd> fds;

	struct pollfd server_pollfd = {_fd, POLLIN, 0};
	fds.push_back(server_pollfd);

	while (true)
	{
		if (poll(fds.data(), fds.size(), -1) == -1)
		{
			perror("poll");
			break;
		}

		for (size_t i = 0; i < fds.size(); ++i)
		{
			if (fds[i].revents & POLLIN)
			{
				if (fds[i].fd == _fd) // New connection
				{
					struct sockaddr_storage client_addr;
					socklen_t addr_size = sizeof(client_addr);
					int client_fd = accept(_fd, (struct sockaddr*)&client_addr, &addr_size);
					if (client_fd == -1)
					{
						perror("accept");
						continue;
					}

					std::cout << "New connection on fd: " << client_fd << std::endl;

					Client new_client(client_fd);
					_clients.push_back(new_client);

					struct pollfd client_pollfd = {client_fd, POLLIN, 0};
					fds.push_back(client_pollfd);
				}
				else // Existing connection
				{
					char buffer[1024];
					int bytes_received = recv(fds[i].fd, buffer, sizeof(buffer), 0);
					if (bytes_received <= 0)
					{
						if (bytes_received == 0)
						{
							std::cout << "Client disconnected: " << fds[i].fd << std::endl;
						}
						else
						{
							perror("recv");
						}
						close(fds[i].fd);
						fds.erase(fds.begin() + i);
						--i;
					}
					else
					{
						buffer[bytes_received] = '\0';
						std::cout << "Received: " << buffer << " from fd: " << fds[i].fd << std::endl;
						handleCommand(buffer, fds[i].fd);
					}
				}
			}
		}
	}
}
