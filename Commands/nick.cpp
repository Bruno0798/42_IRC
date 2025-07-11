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
	const std::string invalidChars = " ,*?!@.:#";
	if (nickname.empty() || invalidChars.find(nickname[0]) != std::string::npos)
		return false;
	for (std::string::const_iterator it = nickname.begin(); it != nickname.end(); ++it)
	{
		if (invalidChars.find(*it) != std::string::npos)
			return false;
	}
	return true;
}

void Server:: handleNick(int client_fd, const std::string& message)
{
	std::istringstream iss(message);
	std::string cmd, nickname, response;
	iss >> cmd >> nickname;

	nickname = nickname.substr(0, 20);
	std::vector<Client>::iterator	client_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
	if (nickname.empty())
	{
		std::cerr << RED << "No Nickname received" << WHITE << std::endl;
		send(client_fd, ERR_NONICKNAMEGIVEN(client_it->getNickname()).c_str(), ERR_NONICKNAMEGIVEN(client_it->getNickname()).size(), 0);
	}
	else if (!isNicknameValid(nickname))
	{
		std::cerr << RED << "Invalid Chars in the nickname" << WHITE << std::endl;
		send(client_fd, ERR_ERRONEUSNICKNAME(client_it->getNickname(), nickname).c_str(), ERR_ERRONEUSNICKNAME(client_it->getNickname(), nickname).size(), 0);
	}
	else if (client_it != _clients.end())
	{
		for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
		{
			std::string nickcpy = nickname;
			std::string nickUsercpy = it->getNickname();
			std::transform(nickcpy.begin(), nickcpy.end(), nickcpy.begin(), ::tolower);
			std::transform(nickUsercpy.begin(), nickUsercpy.end(), nickUsercpy.begin(), ::tolower);
			if (nickUsercpy == nickcpy)
			{
				send(client_fd, ERR_NICKNAMEINUSE(client_it->getNickname(), nickname).c_str(), ERR_NICKNAMEINUSE(client_it->getNickname(), nickname).size(), 0);
				return;
			}
		}
		if (client_it->isRegistered())
		{
			response = ":" + client_it->getNickname() + "!~" + client_it->getUsername() + "@localhost NICK :" + nickname + "\r\n";
			broadcastMessageToClients(response);
		}
		client_it->setNickname(nickname);
	}
	else
		std::cerr << "Client not found for fd: " << client_fd << std::endl;
}