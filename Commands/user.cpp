#include "../Irc.hpp"
#include "../Server.hpp"
#include "../Client.hpp"

/**
 * @brief The USER command is used at the beginning of a connection to specify
 * 		  the username and realname of a new user.
 *
 * 	The minimum length of <username> is 1, ie. it MUST NOT be empty.
 * 	If it is empty, the server SHOULD reject the command with ERR_NEEDMOREPARAMS.
 *
 *  If a client tries to send the USER command after they have already completed
 *  registration with the server, the ERR_ALREADYREGISTERED reply should be sent
 *  and the attempt should fail.
 *
 *  SYNTAX : USER <username> 0 * <realname>
 *
 * 	Numeric Replies:
 * 		ERR_NEEDMOREPARAMS (461)
 * 		ERR_ALREADYREGISTERED (462)
 */

std::string trimLeadingSpaces(const std::string& str)
{
	size_t start = 0;
	while (start < str.size() && std::isspace(str[start])) {
		start++;
	}
	return str.substr(start);
}

void Server::handleUser(int client_fd, const std::string& message)
{
	std::istringstream iss(message);
	std::string cmd, username, hostname, servername, realname;
	iss >> cmd >> username >> hostname >> servername;
	std::getline(iss, realname);
	realname = trimLeadingSpaces(realname);

	std::vector<Client>::iterator client_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
	if(client_it->isRegistered())
		send(client_fd, ERR_ALREADYREGISTERED(client_it->getNickname()).c_str(), ERR_ALREADYREGISTERED(client_it->getNickname()).size(), 0);
	else if (username.empty() || hostname != "0" || servername != "*" || realname.empty() || realname[0] != ':')
		send(client_fd, ERR_NEEDMOREPARAMS(client_it->getNickname(), "USER").c_str(), ERR_NEEDMOREPARAMS(client_it->getNickname(), "USER").length(), 0);
	else if (client_it != _clients.end())
	{
		realname = realname.substr(1);
		client_it->setUserName(username);
		client_it->setRealName(realname);
	}
	else
		std::cerr << "Client not found for fd: " << client_fd << std::endl;
}
