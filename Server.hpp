#ifndef SERVER_H
#define SERVER_H

#include "Irc.hpp"
#include "Client.hpp"
#include "Channel.hpp"

class Server
{
	private:
		struct addrinfo _address;
		struct addrinfo *_servinfo;
		int _port;
		int _fd;
		int _clientFd;
		std::string _password;
		std::vector<Client> _clients;
		std::map<std::string, Channel> _channels;

	public:
		Server();
		Server(int _port, std::string& password);
		~Server();

		void setAddress();
		bool fillServerInfo(char *port);
		bool initServer();
		void runServer();
		void handleCommand(const std::string& command, int client_fd);
		void handlePing(int client_fd, const std::string& message);
		void handleJoin(int client_fd, const std::string& message);
		void handleWho(int client_fd, const std::string& message);
		void handlePrivmsg(int client_fd, const std::string& message);
		void parseClientInfo(const std::string& buffer, int client_fd);
		void handleCommand(Client &user, int client_fd);
		void handleNick(int client_fd, const std::string& message);
		void handlePass(int client_fd, const std::string& message);
		void handleUser(int client_fd, const std::string& message);
		void parseClientInfo(Client &user, int client_fd);
		int getClientFdByName(const std::string& nickname);

		//------------- Diogo ----------------
		void							makeUserList(std::string channel);
		void							broadcastMessageToChannel(const std::string& message, std::string channel);
		std::vector<Client>::iterator	getClient(int clientFd);
		void							checkCommandPart(std::istringstream &lineStream);
		void							commandPart(std::string &channelName);
		bool							LookClientInChannel(std::string channel);
		std::string						getChannelTopic(std::string channel);
		void							changeChannelTopic(std::string &channel, std::string &newTopic);
		void							checkCommandTopic(std::istringstream &lineStream);
		void							commandTopic(std::string &channelName, std::string &newTopic);
		void							removeClientsFromChannels(int clientFd);

		//-------------------------------------

		// ------------ Biltes ----------------
		void handleKick(int client_fd, const std::string& message);
		void handleInvite(int client_fd, const std::string& message);
		void handleMode(int client_fd, const std::string& message);



		void welcome_messages(Client &user);

		class ClientFdMatcher {
		public:
			ClientFdMatcher(int fd) : _fd(fd) {}

		bool operator()(const Client& client) const {
			return client.getFd() == _fd;
		}
	private:
		int _fd;
	};
};

#endif