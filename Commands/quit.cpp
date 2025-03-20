#include "../Irc.hpp"
#include "../Client.hpp"
#include "../Server.hpp"
#include <cctype>

void Server::commandQuit(std::vector<struct pollfd>& fds, size_t i, std::istringstream &msg)
{
	std::string reason;
	msg >> reason;

	reason = getFullMsg(reason, msg);

	if (reason.length() > 1 && !std::isprint(reason[1]))
		handleClientDisconnection(fds, i, 0, "Has left");
	else
		handleClientDisconnection(fds, i, 0, reason);
}
