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
	std::cout << "DEBUG: MESSAGE" << message << std::endl;
	std::string cmd, password;
	iss >> cmd >> password;

	std::cout << "DEBUG: PASSWORD: " << password << std::endl;
	std::vector<Client>::iterator client_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));

	if (password.empty())
	{
		std::string response = ":localhost 461 " + client_it->getNickname() + ":Not enough parameters\r\n";
		send(client_fd, response.c_str(), response.size(), 0);
		return;
	} else if(client_it->isAuth())
	{
		std::string response = ":localhost 462 " + client_it->getNickname() + ":You may not reregister\r\n";
		send(client_fd, response.c_str(), response.size(), 0);
	}
	else if (client_it != _clients.end())
	{
		if(password == _password)
		{
			client_it->setPassword(password);
			std::cout << "DEBUG: Password correct" << std::endl;
			client_it->setAuth(true);
			return ;
		}
		std::string response = ":localhost 464 " + client_it->getNickname() + ":Password incorrect\r\n";
		send(client_fd, response.c_str(), response.size(), 0);
		client_it->delete_buffer();
	}
	else
	{
		std::cerr << "Client not found for fd: " << client_fd << std::endl;
	}
}
