#include "Irc.hpp"
#include "Server.hpp"

int check_args(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cerr << "Correct usage is ./ircserv [port] [password] :)" << std::endl;
		exit(1);
	}

	char *endPtr;
	int num = std::strtol(argv[1], &endPtr, 10);

	if (*endPtr != '\0')
	{
		std::cerr << "Port is not a valid number" << std::endl;
		exit(1);
	}
	if (num < 1024 || num > 65535)
	{
		std::cerr << "Port number invalid. Use a number between 1024 and 65535." << std::endl;
		exit(1);
	}

	return num;
}


int main(int argc, char **argv)
{
	int port = check_args(argc, argv);
	std::string password(argv[2]); // Converts password to std::string

	Server server(port, password);

	// Set up server
	server.setAddress();
	if (!server.fillServerInfo(argv[1]))
	{
		std::cerr << "Failed to fill server info." << std::endl;
		exit(1);
	}

	// Initialize and run server
	if (!server.initServer())
	{
		std::cerr << "Server initialization failed." << std::endl;
		exit(1);
	}

	server.runServer();

	return 0;
}