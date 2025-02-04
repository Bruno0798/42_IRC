#include "Irc.hpp"
#include "Server.hpp"

int check_args(int argc, char **argv)
{
	char *endPtr;
	int num;

	if(argc != 3)
		std::cerr << "Correct usage is ./ircserv [port] [password] :)" << std::endl, exit(1);
	num = std::strtol(argv[1], &endPtr, 10);
	if(endPtr == NULL)
		std::cerr << "Port not a number" << std::endl, exit(1);
//	if(num < 1024 || num > 65535)
//		std::cerr << "Port number invalid Use between 1024 and 65535" << std::endl, exit(1);
	return num;
}


int main(int argc, char **argv)
{
	int port = check_args(argc, argv);
	std::string str(argv[2]); // converts to std::string
	Server server(port, str);

	server.setAddress();
	if(!server.fillServerInfo(argv[1]))
		exit(1);
	if (server.initServer())
	{
	    server.runServer();
	}

	return 0;
}