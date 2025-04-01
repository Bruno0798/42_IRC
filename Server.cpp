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
#include "numerical_replies.hpp"

Server::Server() {}
Server::Server(int port, std::string& password) : _port(port), _password(password) {}
Server::~Server() {}

bool getHostname(std::string &hostname)
{
    std::ifstream file("/proc/sys/kernel/hostname");
    if (!file.is_open()) {
        std::cerr << "Failed to open /proc/sys/kernel/hostname" << std::endl;
        return false;
    }
    std::getline(file, hostname);
    file.close();
    return true;
}
bool Server::fillServerInfo(char *port)
{
    std::string hostname;
    if (!getHostname(hostname))
    {
        std::perror("gethostname");
        return false;
    }

    hostent *host = gethostbyname(hostname.c_str());
    if (host == NULL)
    {
        std::perror("gethostbyname");
        return false;
    }
    char ipstr[INET6_ADDRSTRLEN];
    void *addr;
    if (host->h_addrtype == AF_INET) {
        addr = host->h_addr_list[0];
        inet_ntop(AF_INET, addr, ipstr, sizeof(ipstr));
    } else {
        addr = host->h_addr_list[0];
        inet_ntop(AF_INET6, addr, ipstr, sizeof(ipstr));
    }
    std::memset(&_address, 0, sizeof(_address));
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
		std::perror("socket");
		return false;
	}

	int yes = 1;
	if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
	{
		std::perror("setsockopt");
		return false;
	}

	if (bind(_fd, _servinfo->ai_addr, _servinfo->ai_addrlen) == -1)
	{
		close(_fd);
		std::perror("bind");
		freeaddrinfo(_servinfo);
		return false;
	}
	freeaddrinfo(_servinfo);

	if (listen(_fd, 10) == -1)
	{
		std::perror("listen");
		return false;
	}

	return true;
}

void Server::removeClientsFromChannels(int clientFd, const std::string &msg)
{
	std::map<std::string, Channel>::iterator channel = _channels.begin();

	while (channel != _channels.end())
	{
		if (LookClientInChannel(channel->first))
		{
			channel->second.revokePermissions(_clientFd);
			channel->second.removeClient(_clientFd);
			if (channel->second.getClients().empty())
			{
				std::map<std::string, Channel>::iterator toErase = channel;
				++channel;
				_channels.erase(toErase);
				continue;
			}
		}
		++channel;
	}
	std::string leaveMsg = ":" + getClient(clientFd)->getNickname() + "!" +getClient(clientFd)->getUsername() + "@localhost QUIT :Quit: " + msg + "\r\n";
	broadcastMessageToClients(leaveMsg);
}




void Server::runServer()
{
	std::vector<pollfd> fds;
	pollfd server_pollfd = {_fd, POLLIN, 0};
	fds.push_back(server_pollfd);

	std::cout << GREEN << "Server Online!" << WHITE << std::endl;

	while (!shut_down)
	{
		int poll_count = poll(fds.data(), fds.size(), -1);
		if (poll_count == -1)
		{
			std::perror("poll");
			break;
		}
		for (size_t i = 0; i < fds.size(); ++i)
		{
			_clientFd = fds[i].fd;
			if (fds[i].revents & POLLIN)
			{
				if (fds[i].fd == _fd)
					handleNewConnection(fds);
				else
				{
					if(!handleClientData(fds, i))
						;
				}
			}
			else if (fds[i].revents & POLLOUT)
				handleClientWrite(fds, i);
			else if (fds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
				handleClientError(fds, i);
		}
	}
    for (size_t i = 0; i < fds.size(); ++i)
    {
        if (fds[i].fd > 0)
            close(fds[i].fd);
    }
}

void Server::handleNewConnection(std::vector<pollfd>& fds)
{
	sockaddr_storage client_addr;
	socklen_t addr_size = sizeof(client_addr);
	int client_fd = accept(_fd, reinterpret_cast<struct sockaddr*>(&client_addr), &addr_size);
	if (client_fd == -1)
	{
		std::perror("accept");
		return;
	}
	std::cout << GREEN << "New Connection on fd: " << client_fd << std::endl;

	Client new_client(client_fd);
	_clients.push_back(new_client);

	pollfd client_pollfd = {client_fd, POLLIN, 0};
	fds.push_back(client_pollfd);
}

bool Server::handleClientData(std::vector<pollfd>& fds, size_t i)
{
	char buffer[1024];
	const int bytes_received = recv(fds[i].fd, buffer, sizeof(buffer), 0);

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

void Server::handleClientDisconnection(std::vector<pollfd>& fds, size_t i, int bytes_received, const std::string &leaveMsg)
{
	if (bytes_received == 0)
	{
		removeClientsFromChannels(fds[i].fd, leaveMsg);
		std::cout << RED <<  "Client disconnected: " << fds[i].fd << WHITE << std::endl;
	}
	else
		std::perror("recv");
	close(fds[i].fd);
	fds.erase(fds.begin() + i);
	_clients.erase(_clients.begin() + (i - 1));
	--i;
}

void Server::handleClientWrite(std::vector<pollfd>& fds, size_t i)
{
	Client &user = _clients.at(i - 1);
	std::istringstream check(user.getBuffer());
	std::string cmd, msg;
	check >> cmd;
	std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
	if (cmd == "QUIT")
		commandQuit(fds, i, check);
	else
		handleCommand(user, fds[i].fd);
	fds[i].events = POLLIN;
}

void Server::handleClientError(std::vector<pollfd>& fds, size_t i)
{
	std::cerr << "Error on fd: " << fds[i].fd << std::endl;
	close(fds[i].fd);
	fds.erase(fds.begin() + i);
	_clients.erase(_clients.begin() + (i - 1));
	--i;
}

void Server::welcome_messages(int client_fd)
{
	time_t _server_creation_time = std::time(NULL);
	std::string time = std::asctime(std::localtime(&_server_creation_time));
	time.erase(std::remove(time.begin(), time.end(), '\n'), time.end());

	std::vector<Client>::iterator client_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
	if (client_it == _clients.end())
	{
		std::cerr << "Client not found for fd: " << client_fd << std::endl;
		return;
	}

	Client &user = *client_it;
	std::string msg = ":localhost 372 " + client_it->getNickname() + " :WELCOME COMRADE, TO OUR IRC SERVER!\r\n";
	std::string msgEnd = ":localhost 376 " + client_it->getNickname() + " :End of /MOTD command.\r\n";

	send(user.getFd(), RPL_WELCOME(user.getNickname()).c_str(), RPL_WELCOME(user.getNickname()).size(), 0);
	send(user.getFd(), RPL_YOURHOST(user.getNickname()).c_str(), RPL_YOURHOST(user.getNickname()).size(), 0);
	send(user.getFd(), RPL_CREATED(user.getNickname(), time).c_str(), RPL_CREATED(user.getNickname(),time).size(), 0);
	send(user.getFd(), RPL_MYINFO(user.getNickname()).c_str(), RPL_MYINFO(user.getNickname()).size(), 0);
	send(user.getFd(), RPL_ISUPPORT(user.getNickname()).c_str(), RPL_ISUPPORT(user.getNickname()).size(), 0);
	send(user.getFd(), msg.c_str(), msg.size(), 0);
	send(user.getFd(), msgEnd.c_str(), msgEnd.size(), 0);
}

Server::ClientFdMatcher::ClientFdMatcher(int fd) : _fd(fd)
{

}

bool Server::ClientFdMatcher::operator()(const Client &client) const {
	return client.getFd() == _fd;
}
