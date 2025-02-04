#include "Client.hpp"

Client::Client()
		: _fd(-1), _registered(false)
{
	std::cout << "Client Default Constructor Called" << std::endl;
}


Client::Client(int fd, const std::string& address, int port)
		: _fd(fd), _ipAddress(address), _port(port) {}

Client::~Client()
{
	std::cout << "Client Destructor Called" << std::endl;
}

// GETTERS

int Client::getFd() const
{
	return _fd;
}

std::string Client::getNickname() const
{
	return _nickname;
}

std::string Client::getUsername() const
{
	return _username;
}

std::string Client::getRealname() const
{
	return _realname;
}

bool Client::isRegistered() const
{
	return _registered;
}

//setters

void Client::setFd(int fd)
{
	_fd = fd;
}

void Client::setIpAddress(const std::string& ip)
{
	_ipAddress = ip;
}


void Client::setNickname(const std::string nickname)
{
	_nickname = nickname;
}

void Client::setUserName(const std::string username)
{
	_username = username;
}

void Client::setRealName(const std::string realname)
{
	_realname = realname;
}

void Client::setRegistered(bool registered)
{
	_registered = registered;
}