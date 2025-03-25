#include "Irc.hpp"
#include "Server.hpp"

bool shut_down = false;

int check_args(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cerr << RED << "Correct usage is ./ircserv [port] [password] :)" << WHITE << std::endl;
		std::exit(1);
	}

	char *endPtr;
	int num = std::strtol(argv[1], &endPtr, 10);

	if (*endPtr != '\0')
	{
		std::cerr << RED << "Port is not a valid number" << WHITE << std::endl;
		std::exit(1);
	}
	if (num < 1024 || num > 65535)
	{
		std::cerr << RED << "Port number invalid. Use a number between 1024 and 65535." << WHITE << std::endl;
		std::exit(1);
	}

	return num;
}

void signalHandler(int signum)
{
	std::cout << "Interrupt signal (" << signum << ") received.\n";
	if (signum == 13)
		return;
	shut_down = true;
}

int main(int argc, char **argv)
{
	signal(SIGINT, signalHandler);
	signal(SIGQUIT, signalHandler);
	signal(SIGPIPE, signalHandler);
	std::system("clear");

	int port = check_args(argc, argv);
	std::string password(argv[2]);

	Server server(port, password);

	if (!server.fillServerInfo(argv[1]))
	{
		std::cerr << RED << "Failed to fill server info." << WHITE << std::endl;
		std::exit(1);
	}

	// Initialize and run server
	if (!server.initServer())
	{
		std::cerr << RED << "Server initialization failed." << WHITE << std::endl;
		std::exit(1);
	}
	server.runServer();
	return 0;
}
