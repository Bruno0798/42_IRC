#include "Irc.hpp"
#include "Server.hpp"

Server::Server()
{
	std::cout << "Server Default Constructor Called" << std::endl;
}

Server::Server(int port, std::string& password): _servinfo(NULL), _port(port), _password(password)
{
	std::cout << YELLOW << "Server running ..." << WHITE << std::endl;
	memset(&_address, 0, sizeof(_address));
}

Server::~Server()
{
	std::cout << "Server Destructor Called" << std::endl;
}


void Server::setAddress()
{
	_address.ai_family = AF_INET; // set the address family to IPV4
	_address.ai_socktype = SOCK_STREAM; //TCP stream sockets
	_address.ai_flags = AI_PASSIVE; // start in localhost by default
}


bool Server::fillServerInfo(char *port)
{
	int status = getaddrinfo(NULL, port, &_address, &_servinfo);
	if (status != 0)
	{
		std::cerr << RED << "[Server] Address Failed: "
				  << gai_strerror(status) << WHITE << std::endl;
		return false;
	}
	return true;
}


bool Server::initServer()
{
	_fd = socket(_servinfo->ai_family, _servinfo->ai_socktype, _servinfo->ai_protocol);
	if(_fd == -1)
	{
		std::cerr << RED << "[Server] Socket Failed" << WHITE <<  std::endl;
		return false;
	}
	int optvalue = 1;
	if(setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &optvalue, sizeof (optvalue)) == -1)
	{
		std::cerr << RED << "[Server] Impossible to reuse" << WHITE<< std::endl;
		return false;
	}
	if (bind(_fd, _servinfo->ai_addr, _servinfo->ai_addrlen) == -1)
	{
		std::cerr << RED << "[Server] Port already in use" << WHITE << std::endl;
		return false;
	}
	if (listen(_fd, 10) == -1)
	{
		std::cerr << RED << "[Server] Listen failed" << WHITE << std::endl;
		return false;
	}
	freeaddrinfo(_servinfo);
	return true;
}

void Server::runServer()
{
	struct sockaddr_storage client_addr;
	socklen_t addr_size = sizeof(client_addr);
	int client_fd;
	std::vector<struct pollfd> fds;

	std::cout << GREEN << "[Server] Waiting for connections..." << WHITE << std::endl;

	struct pollfd server_fd;
	server_fd.fd = _fd;
	server_fd.events = POLLIN;
	fds.push_back(server_fd);

	while (true)
	{
		int poll_count = poll(fds.data(), fds.size(), -1);
		if (poll_count == -1)
		{
			std::cerr << RED << "[Server] Poll failed: " << strerror(errno) << WHITE << std::endl;
			break;
		}

		for (size_t i = 0; i < fds.size(); ++i)
		{
			if (fds[i].revents & POLLIN)
			{
				if (fds[i].fd == _fd)
				{
					client_fd = accept(_fd, (struct sockaddr *)&client_addr, &addr_size);
					if (client_fd == -1)
					{
						std::cerr << RED << "[Server] Accept failed: " << strerror(errno) << WHITE << std::endl;
						continue;
					}

					struct sockaddr_in *addr = (struct sockaddr_in *)&client_addr;
					std::cout << GREEN << "[Server] Connection accepted from "
							  << inet_ntoa(addr->sin_addr) << WHITE << std::endl;

					struct pollfd client_pollfd;
					client_pollfd.fd = client_fd;
					client_pollfd.events = POLLIN;
					fds.push_back(client_pollfd);
				}
				else
				{
					char buffer[1024];
					ssize_t bytes_received = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);
					if (bytes_received <= 0)
					{
						std::cout << "[Server] Client disconnected: " << _clients[fds[i].fd].getNickname() << std::endl;
						close(fds[i].fd);
						fds.erase(fds.begin() + i);
						--i;
					}
					buffer[bytes_received] = '\0';
					std::cout << YELLOW << "[Server] Received: " << buffer << WHITE << std::endl;

					std::string command(buffer);
					command.erase(command.find_last_not_of(" \r\n") + 1); // Trim trailing whitespace
					handleCommand(fds[i].fd, command);
				}
			}
		}
	}
}


void Server::handleCommand(int client_fd, const std::string& command)
{
	if (command.compare(0, 4, "NICK") == 0)

	{
		std::string nickname = command.substr(5); // Extract nickname
		_clients[client_fd].setNickname(nickname);
		std::cout << "[Server] Client set nickname: " << nickname << std::endl;
	}
	else if (command.compare(0, 4, "USER") == 0)

	{
		// Parse and set username/realname
	}
	else if (_clients[client_fd].isRegistered())
	{
		// Handle other commands like PRIVMSG or JOIN
	}
	else
	{
		std::string response = "You must register first!\n";
		send(client_fd, response.c_str(), response.size(), 0);
	}
}
