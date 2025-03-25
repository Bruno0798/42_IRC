#include "Irc.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include "Channel.hpp"

void Server::checkRegist(int client_fd)
{
	std::vector<Client>::iterator client_it = std::find_if(_clients.begin(), _clients.end(), ClientFdMatcher(client_fd));
	if(client_it->getNickname() != "*" && !client_it->getUsername().empty())
    {
		client_it->setRegistered(true);
		welcome_messages(client_fd);
	}
}

void Server::handleCommand(Client& user, int client_fd)
{
	std::istringstream iss(user.getBuffer());
	std::string line, cmds;
	_clientFd = client_fd;

	while(getline(iss, line))
	{
		std::istringstream cmd(line);
		cmd >> cmds;
		std::string cmdsCpy = cmds;
		std::transform(cmdsCpy.begin(), cmdsCpy.end(), cmdsCpy.begin(), ::toupper);
		std::cout << GREEN << "RECEIVED:" << line << WHITE << std::endl;
		if(cmdsCpy == "CAP" || cmds == "WHO")
			;
		else if(!user.isAuth())
		{
			if (cmdsCpy == "PASS") handlePass(client_fd, line);
			else
			{
				std::cout << RED << "ERROR: NOT AUTHENTICATED" << WHITE << std::endl;
				send(client_fd, ERR_NOTAUTHENTICATED(user.getNickname()).c_str(), ERR_NOTAUTHENTICATED(user.getNickname()).size(), 0);
			}
		} else if(!user.isRegistered())
		{
			if (cmdsCpy == "PASS") handlePass(client_fd, line);
			else if (cmdsCpy =="NICK") handleNick(client_fd, line);
			else if (cmdsCpy == "USER") handleUser(client_fd, line);
			else send(client_fd, ERR_NOTREGISTERED(user.getNickname()).c_str(), ERR_NOTREGISTERED(user.getNickname()).size(), 0);
			checkRegist(client_fd);
		} else
		{
			if (cmdsCpy =="PASS") handlePass(client_fd, line);
			else if (cmdsCpy =="NICK") handleNick(client_fd, line);
			else if (cmdsCpy == "USER") handleUser(client_fd, line);
			else if (cmdsCpy == "JOIN") checkCommandJoin(client_fd, cmd);
			else if (cmdsCpy == "PING") handlePing(client_fd, line);
			else if (cmdsCpy == "MODE") handleMode(client_fd, line);
			else if (cmdsCpy == "TOPIC") checkCommandTopic(cmd);
			else if (cmdsCpy == "KICK") handleKick(client_fd, line);
			else if (cmdsCpy == "INVITE") handleInvite(client_fd, line);
			else if (cmdsCpy =="PRIVMSG") handlePrivmsg(client_fd, line);
			else if (cmdsCpy =="PART") checkCommandPart(cmd);
			else
			{
				std::string errMsg = ":localhost 421 " + user.getNickname() + " " + cmds + " :Unknown commands\r\n";
				send(_clientFd, errMsg.c_str(), errMsg.size(), 0);
			}
		}
	}
	user.delete_buffer();
}

int Server::getClientFdByName(const std::string& nickname)
{
	for (std::vector<Client>::iterator clientIt = _clients.begin(); clientIt != _clients.end(); ++clientIt) {
		if (clientIt->getNickname() == nickname)
			return clientIt->getFd();
	}
	return -1;
}
