#include "Server.hpp"

#define blue "\033[34m"
#define red "\033[31m"
#define green "\033[32m"
#define yellow "\033[33m"
#define magenta "\033[35m"
#define cyan "\033[36m"
#define reset "\033[0m"

Server::Server()
{
	std::cout << "Server Default Constructor Called" << std::endl;
}

Server::Server(int port, std::string& password): _servinfo(NULL), _port(port), _password(password)
{
	std::cout << yellow << "Server running ..." << reset << std::endl;
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
	std::cout << port << std::endl;
	if (getaddrinfo(NULL, port, &_address, &_servinfo) < 0)
	{
		std::cerr << red << "[Server] Flop addrinfo" << reset << std::endl;
		return (false);
	}
	return (true);
}

