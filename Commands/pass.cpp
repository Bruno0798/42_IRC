#include "../Irc.hpp"
#include "../Client.hpp"
#include "../Server.hpp"

/**
* @brief The PASS command is used to set a ‘connection password’.
 * The password supplied must match the one defined in the server configuration.
 *
 * Syntax: PASS <password>
 *
 * Numeric replies:
 * 	ERR_NEEDMOREPARAMS (461)
 * 	ERR_ALREADYREGISTERED (462)
 * 	ERR_PASSWDMISMATCH (464)
 *
 * Example :
 *  [CLIENT] /PASS secretpassword
 */

void Server::handlePass(int client_fd, const std::string& message)
{
	std::istringstream iss(message);
	std::string cmd, password;
	iss >> cmd >> password;

	if (password.empty())
	{
		std::cerr << "PASS command requires a password" << std::endl;
		return;
	}

	std::vector<Client>::iterator client_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
	if (client_it != _clients.end())
	{
		client_it->setPassword(password);
		std::string response = ":localhost 001 " + client_it->getNickname() + " :Password set\r\n";
		send(client_fd, response.c_str(), response.size(), 0);
	}
	else
	{
		std::cerr << "Client not found for fd: " << client_fd << std::endl;
	}
}
