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
	if (getaddrinfo(NULL, port, &_address, &_servinfo) < 0)
	{
		std::cerr << RED << "[Server] Address Failed" << WHITE << std::endl;
		return (false);
	}
	return (true);
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

}
