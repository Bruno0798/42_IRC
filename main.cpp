#include <iostream>
#include <cstdlib>
#include <sstream>
#include "Server.hpp"

#define blue "\033[34m"
#define red "\033[31m"
#define green "\033[32m"
#define yellow "\033[33m"
#define magenta "\033[35m"
#define cyan "\033[36m"
#define reset "\033[0m"

int check_args(int argc, char **argv)
{
	char *endPtr;
	int num;

	if(argc != 3)
		std::cerr << "Correct usage is ./ircserv [port] [password] :)" << std::endl, exit(1);
	num = std::strtol(argv[1], &endPtr, 10);
	if(endPtr == NULL)
		std::cerr << "Port not a number" << std::endl, exit(1);
	return num;
}


int main(int argc, char **argv)
{
	std::string str(argv[2]); // converts to std::string
	Server server(check_args(argc, argv), str);

	server.setAddress();
	if(!server.fillServerInfo(argv[1]))
		exit(1);

	return 0;
}