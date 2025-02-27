#include "../Irc.hpp"
#include "../Client.hpp"
#include "../Server.hpp"

/**
 * @brief The NICK command is used to give the client a nickname or
 * 		change the previous one.
 *
 * 	Syntax: NICK <nickname>
 *
 * 	Nicknames are non-empty strings with the following restrictions:
 *
 * 	They MUST NOT contain any of the following characters:
 * 	- space (' '),
 * 	- comma (','),
 * 	- asterisk ('*'),
 * 	- question mark ('?'),
 * 	- exclamation mark ('!'),
 * 	- at sign ('@'),
 * 	- dot ('.').
 *
 * 	They MUST NOT start with any of the following characters:
 * 	dollar ('$'), colon (':'), diese (#).
 *
 * 	Numeric Replies:
 *
 * 	ERR_NONICKNAMEGIVEN (431)
 * 	ERR_ERRONEUSNICKNAME (432)
 * 	ERR_NICKNAMEINUSE (433)
 * 	ERR_NICKCOLLISION (436)
 *
 * 	Example:
 * 	[CLIENT] /Nick mike
 *
 */

void Server::handleNick(int client_fd, const std::string& message)
{
	std::istringstream iss(message);
	std::string cmd, nickname;
	iss >> cmd >> nickname;

	if (nickname.empty())
	{
		std::cerr << "NICK command requires a nickname" << std::endl;
		return;
	}

	std::vector<Client>::iterator client_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
	if (client_it != _clients.end())
	{
		client_it->setNickname(nickname);
		std::string response = ":localhost 001 " + nickname + " :Nickname set to " + nickname + "\r\n";
		send(client_fd, response.c_str(), response.size(), 0);
	}
	else
	{
		std::cerr << "Client not found for fd: " << client_fd << std::endl;
	}
}