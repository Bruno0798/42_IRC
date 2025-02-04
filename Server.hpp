#ifndef SERVER_H
#define SERVER_H

#include "Irc.hpp"
#include "Client.hpp"

class Server
{
private:
	struct addrinfo _address;
	struct addrinfo *_servinfo;
	int _port;
	int _fd;
	std::string _password;
	std::vector<Client> _clients;
public:
	Server();
	Server(int _port, std::string& password);
	~Server();

	void setAddress();
	bool fillServerInfo(char *port);
	bool initServer();
	void runServer();
	void handleCommand(int client_fd, const std::string& command);

	struct ClientFdMatcher {
		int client_fd;
		ClientFdMatcher(int fd) : client_fd(fd) {}
		bool operator()(const Client& client) const {
			return client.getFd() == client_fd;
		}
	};
};

#endif