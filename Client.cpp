#include "Client.hpp"

Client::Client()
		: _fd(-1), _port(), _registered(false), _authenticated(false)
{
	// std::cout << "Client Default Constructor Called" << std::endl;
}

Client::Client(int fd)
		: _fd(fd), _port(), _registered(false), _authenticated(false) // Initialize other members if necessary
{
	// std::cout << "Client Constructor with fd Called" << std::endl;
}

Client::Client(int fd, const std::string& address, int port)
		: _fd(fd), _ipAddress(address), _port(port), _registered(false) {}

Client::~Client()
{
	// std::cout << "Client Destructor Called" << std::endl;
}

// Getters
int Client::getFd() const
{
	return _fd;
}

std::string Client::getNickname() const
{
	if(_nickname.empty())
		return "*";
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

std::string Client::getPassword() const
{
	return _password;
}


bool Client::isRegistered() const
{
	return _registered;
}

// Setters
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

void Client::setPassword(const std::string password)
{
	_password = password;
}

bool Client::authenticate()
{
	if(_nickname.empty() || _username.empty() || _password.empty())
		return false;
	return true;

}
void Client::setBuffer(char *buf)
{
	_buffer += buf;
}

void Client::delete_buffer()
{
	_buffer.clear();
}

std::string Client::getBuffer() const
{
	return _buffer;
}

void Client::setAuth(bool auth)
{
	_authenticated = auth;
}

bool Client::isAuth() const
{
	return _authenticated;
}