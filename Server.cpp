#include "Server.hpp"
#include "Irc.hpp"
#include <iostream>
#include <cstring>
#include <iomanip>
#include <arpa/inet.h>
#include <map>
#include <string>
#include <unistd.h>
#include <poll.h>
#include <vector>

Server::Server() {}

Server::Server(int port, std::string& password)
		: _port(port), _password(password) {}

Server::~Server() {
}

bool Server::fillServerInfo(char *port)
{
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == -1)
    {
        perror("gethostname");
        return false;
    }

    struct hostent *host = gethostbyname(hostname);
    if (host == NULL)
    {
        perror("gethostbyname");
        return false;
    }

    // Print the IP address
    char ipstr[INET6_ADDRSTRLEN];
    void *addr;
    if (host->h_addrtype == AF_INET) {
        addr = host->h_addr_list[0];
        inet_ntop(AF_INET, addr, ipstr, sizeof(ipstr));
    } else {
        addr = host->h_addr_list[0];
        inet_ntop(AF_INET6, addr, ipstr, sizeof(ipstr));
    }

    // Use localhost for getaddrinfo
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

    std::cout << " ________  _________         _____  _______      ______  " << std::endl;
    std::cout << "|_   __  ||  _   _  |       |_   _||_   __ \\   .' ___  | " << std::endl;
    std::cout << "  | |_ \\_||_/ | | \\_|         | |    | |__) | / .'   \\_|" << std::endl;
    std::cout << "  |  _|       | |             | |    |  __ /  | |        " << std::endl;
    std::cout << " _| |_       _| |_  _______  _| |_  _| |  \\ \\_\\ `.___.'\\ " << std::endl;
    std::cout << "|_____|     |_____||_______||_____||____| |___|`.____ .' " << std::endl;
    std::cout << "                                                         " << std::endl;

    std::cout << std::endl
        << "\t\tServer Information" << std::endl
        << std::endl
        << "\t" << std::setw(8) << std::left << "IP" << ": " << ipstr << std::endl
        << "\t" << std::setw(8) << std::left << "PORT" << ": " << port << std::endl
        << "\t" << std::setw(8) << std::left << "PASS" << ": " << _password << std::endl
        << std::endl;

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

void Server::removeClientsFromChannels(int clientFd, const std::string &msg)
{
	std::map<std::string, Channel>::iterator channel = _channels.begin();

	while (channel != _channels.end())
	{
		if(LookClientInChannel(channel->first))
		{
			std::string leaveMsg = ":" + getClient(clientFd)->getNickname() + "!" +getClient(clientFd)->getUsername() + "@localhost QUIT :Quit: " + msg + "\r\n";
			channel->second.removeClient(_clientFd);
			send(_clientFd, leaveMsg.c_str(), leaveMsg.size(), 0);
			broadcastMessageToChannel(leaveMsg, channel->first);
		}
		channel++;
	}
}

void Server::runServer()
{
	std::vector<struct pollfd> fds;
	struct pollfd server_pollfd = {_fd, POLLIN, 0};
	fds.push_back(server_pollfd);

	std::cout << "Server is Running..." << std::endl;

	while (!shut_down)
	{
		int poll_count = poll(fds.data(), fds.size(), -1);
		if (poll_count == -1) {
			perror("poll");
			break;
		}
		for (size_t i = 0; i < fds.size(); ++i)
		{
			if (fds[i].revents & POLLIN)
			{
				if (fds[i].fd == _fd)
					handleNewConnection(fds);
				else
				{
					if(!handleClientData(fds, i))
						continue;
				}
			}
			else if (fds[i].revents & POLLOUT)
				handleClientWrite(fds, i);
			else if (fds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
				handleClientError(fds, i);
		}
	}
}


void Server::handleNewConnection(std::vector<struct pollfd>& fds)
{
	struct sockaddr_storage client_addr;
	socklen_t addr_size = sizeof(client_addr);
	int client_fd = accept(_fd, (struct sockaddr*)&client_addr, &addr_size);
	if (client_fd == -1)
	{
		perror("accept");
		return;
	}

	std::cout << "New connection on fd: " << client_fd << std::endl;

	Client new_client(client_fd);
	_clients.push_back(new_client);

	struct pollfd client_pollfd = {client_fd, POLLIN, 0};
	fds.push_back(client_pollfd);
}

bool Server::handleClientData(std::vector<struct pollfd>& fds, size_t i)
{
	char buffer[1024];
	int bytes_received = recv(fds[i].fd, buffer, sizeof(buffer), 0);

	if (bytes_received <= 0)
		handleClientDisconnection(fds, i, bytes_received, "Has left");
	else
	{
		Client &user = _clients.at(i - 1);
		buffer[bytes_received] = '\0';
		user.setBuffer(buffer);
		if(user.getBuffer().find('\n') == std::string::npos)
			return false;
	}
	fds[i].events = POLLOUT;
	return true;
}

void Server::handleClientDisconnection(std::vector<struct pollfd>& fds, size_t i, int bytes_received, const std::string &leaveMsg)
{
	if (bytes_received == 0)
	{
		//leave all channels
		removeClientsFromChannels(fds[i].fd, leaveMsg);
		std::cout << "Client disconnected: " << fds[i].fd << std::endl;
	}
	else
		perror("recv");
	close(fds[i].fd);
	fds.erase(fds.begin() + i);
	_clients.erase(_clients.begin() + (i - 1));
	--i;
}

void Server::handleClientWrite(std::vector<struct pollfd>& fds, size_t i)
{
	Client &user = _clients.at(i - 1);
	std::istringstream check(user.getBuffer());
	std::string cmd, msg;
	check >> cmd >> msg;
	if (cmd == "QUIT")
		commandQuit(fds, i, msg);
	else
	{
		handleCommand(user, fds[i].fd);
		fds[i].events = POLLIN;
	}
}

void Server::handleClientError(std::vector<struct pollfd>& fds, size_t i)
{
	std::cerr << "Error on fd: " << fds[i].fd << std::endl;
	close(fds[i].fd);
	fds.erase(fds.begin() + i);
	_clients.erase(_clients.begin() + (i - 1));
	--i;
}

//
//void Server::parseClientInfo(Client &user, int client_fd)
//{
//	std::istringstream iss(user.getBuffer());
//	std::string token;
//	std::string nickname, username, password;
//
//	while (iss >> token)
//	{
//		if (token == "PASS")
//		{
//			iss >> password;
//			handlePass(client_fd, user.getBuffer());
//		}
//		if (token == "NICK")
//		{
//			iss >> nickname;
//			handleNick(client_fd, user.getBuffer());
//		}
//		if (token == "USER")
//		{
//			iss >> username;
//			handleUser(client_fd, user.getBuffer());
//		}
//	}
//
//	std::cout << "Parsed nickname: " << nickname << ", username: " << username << ", password: " << password << std::endl;
//
//	// Find the client and update its information
//	std::vector<Client>::iterator client_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
//	if (client_it != _clients.end())
//	{
//		if (!nickname.empty())
//			client_it->setNickname(nickname);
//		if (!username.empty())
//			client_it->setUserName(username);
//		if (!password.empty())
//			client_it->setPassword(password);
//
//		std::cout << "Nickname: "<< client_it->getNickname()  << std::endl;
//		std::cout << "UserName: "<< client_it->getUsername()  << std::endl;
//		std::cout << "Password: "<< client_it->getPassword()  << std::endl;
//
//		if (client_it->getPassword() == _password && !user.isAuth())
//		{
//			client_it->setAuth(true);
//			std::cout << "Client authenticated for fd: " << client_fd << std::endl;
//		}
//		else
//			std::cerr << "Invalid password for fd: " << client_fd << std::endl;
//	}
//	else
//		std::cerr << "Client not found for fd: " << client_fd << std::endl;
//}

void Server::welcome_messages(int client_fd)
{
	// Find the client associated with the given fd
	std::vector<Client>::iterator client_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
	if (client_it == _clients.end())
	{
		std::cerr << "Client not found for fd: " << client_fd << std::endl;
		return;
	}

	Client &user = *client_it;
	std::string welcome = ":localhost 001 " + user.getNickname() + " :Welcome to the IRC server\r\n";
	std::string yourHost = ":localhost 002 " + user.getNickname() + " :Your host is localhost, running version 1.0\r\n";
	std::string created = ":localhost 003 " + user.getNickname() + " :This server was created today\r\n";
	std::string myInfo = ":localhost 004 " + user.getNickname() + " localhost 1.0 o o\r\n";
	std::string serverNotice = ":localhost 005 " + user.getNickname() + " :Please note that this server is for testing purposes only\r\n";

	send(user.getFd(), welcome.c_str(), welcome.size(), 0);
	send(user.getFd(), yourHost.c_str(), yourHost.size(), 0);
	send(user.getFd(), created.c_str(), created.size(), 0);
	send(user.getFd(), myInfo.c_str(), myInfo.size(), 0);
	send(user.getFd(), serverNotice.c_str(), serverNotice.size(), 0);
}

std::string Server::getPass()
{
	return _password;
}

