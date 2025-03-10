#include <algorithm>

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

bool isNicknameValid(const std::string& nickname)
{
	const std::string invalidChars = " ,*?!@.";
	if (nickname.empty() || invalidChars.find(nickname[0]) != std::string::npos)
		return false;
	for (std::string::const_iterator it = nickname.begin(); it != nickname.end(); ++it)
	{
		if (invalidChars.find(*it) != std::string::npos)
			return false;
	}
	return true;
}

void Server::handleNick(int client_fd, const std::string& message)
{
	std::istringstream iss(message);
	std::string cmd, nickname, response;
	iss >> cmd >> nickname;

	std::vector<Client>::iterator client_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
	if (nickname.empty())
	{
		if (client_it->isRegistered())
			response = ":localhost 431 :No nickname given\r\n";
		send(client_fd, response.c_str(), response.size(), 0);
		return;
	}
	if (!isNicknameValid(nickname))
	{
		std::string response = ":localhost 432 " + client_it->getNickname() + " " + nickname +  " :Erroneous nickname\r\n";
		send(client_fd, response.c_str(), response.size(), 0);
		return;
	}
	if (client_it != _clients.end())
	{
		// Check if the nickname is already in use
		for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
		{
			if (it->getNickname() == nickname)
			{
				response = ":localhost 433 " + nickname + " :Nickname is already in use\r\n";
				send(client_fd, response.c_str(), response.size(), 0);
				return;
			}
		}
		client_it->setNickname(nickname);
	}
	else
	{
		std::cerr << "Client not found for fd: " << client_fd << std::endl;
	}
}