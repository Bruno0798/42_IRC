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
	if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
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

void Server::runServer()
{
	std::vector<struct pollfd> fds;

	struct pollfd server_pollfd = {_fd, POLLIN, 0};
	fds.push_back(server_pollfd);

	std::cout << GREEN << "Server is Running..." << WHITE << std::endl;

	while (true)
	{
		if (poll(fds.data(), fds.size(), -1) == -1)
		{
			perror("poll");
			break;
		}

		for (size_t i = 0; i < fds.size(); ++i)
		{
			if (fds[i].revents & POLLIN) // If the event that occurred is a POLLIN (aka "data is ready to recv() on this socket")
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
							std::cout << "Client disconnected: " << fds[i].fd << std::endl;
						else
							perror("recv");
						close(fds[i].fd);
						fds.erase(fds.begin() + i);
						--i;
					}
					else
					{
						buffer[bytes_received] = '\0';
						std::cout << "Received: " << buffer << " from fd: " << fds[i].fd << std::endl;
						parseClientInfo(buffer, fds[i].fd);
						handleCommand(buffer, fds[i].fd);
					}
				}
			}
		}
	}
}

void Server::parseClientInfo(const std::string& buffer, int client_fd)
{
	std::istringstream iss(buffer);
	std::string token;
	std::string nickname, username, password;

	// Assuming the buffer contains the nickname, username, and password in a specific format
	// For example: "PASS <password> NICK <nickname> USER <username>"
	while (iss >> token)
	{
		if (token == "PASS")
		{
			iss >> password;
		}
		else if (token == "NICK")
		{
			iss >> nickname;
		}
		else if (token == "USER")
		{
			iss >> username;
		}
	}

	std::cout << "Parsed nickname: " << nickname << ", username: " << username << ", password: " << password << std::endl;

	// Find the client and update its information
	std::vector<Client>::iterator client_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
	if (client_it != _clients.end() && !(nickname.empty() || username.empty()))
	{
		client_it->setNickname(nickname);
		client_it->setUserName(username);
		client_it->setPassword(password);
		if (password == _password)
		{
			client_it->authenticate();
			std::cout << "Client authenticated for fd: " << client_fd << std::endl;
		}
		else
		{
			std::cerr << "Invalid password for fd: " << client_fd << std::endl;
		}
	}
	else
	{
		std::cerr << "Client not found for fd: " << client_fd << std::endl;
	}
}