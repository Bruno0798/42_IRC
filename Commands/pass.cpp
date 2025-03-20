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

	std::vector<Client>::iterator client_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));

	if(client_it->isAuth())
	{
		std::cerr << RED << "Trying to reregister" << WHITE << std::endl;
		send(client_fd, ERR_ALREADYREGISTERED(client_it->getNickname()).c_str(), ERR_ALREADYREGISTERED(client_it->getNickname()).size(), 0);
	}
	else if (password.empty())
	{
		std::cerr << RED << "No Password found" << WHITE << std::endl;
		send(client_fd, ERR_NEEDMOREPARAMS(client_it->getNickname(), "PASS").c_str(), ERR_NEEDMOREPARAMS(client_it->getNickname(), "PASS").size(), 0);
	}
	else if (client_it != _clients.end())
	{
		if(password == _password)
		{
			std::cout << GREEN << "Password Correct!" << WHITE << std::endl;
			client_it->setPassword(password);
			client_it->setAuth(true);
			std::cout << GREEN << "Client " << client_fd << " is now Authenticated" << WHITE << std::endl;
			return ;
		}
		std::cerr << RED << "Password Incorrect" << WHITE << std::endl;
		send(client_fd, ERR_PASSWDMISMATCH(client_it->getNickname()).c_str(), ERR_PASSWDMISMATCH(client_it->getNickname()).size(), 0);
	}
	else
	{
		std::cerr << "Client not found for fd: " << client_fd << std::endl;
	}
}
