#include "../Irc.hpp"
#include "../Client.hpp"
#include "../Server.hpp"
#include <cctype>

void Server::commandQuit(std::vector<struct pollfd>& fds, size_t i, std::string &msg)
{
	if (!std::isprint(msg[1]))
		handleClientDisconnection(fds, i, 0, "Has left");
	else
		handleClientDisconnection(fds, i, 0, msg);
}




	
