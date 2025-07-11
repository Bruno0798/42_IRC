#ifndef SERVER_H
#define SERVER_H

#include "Irc.hpp"
#include "Client.hpp"
#include "Channel.hpp"


extern bool	shut_down;

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
		void parseClientInfo(const std::string& buffer, int client_fd);
		void parseClientInfo(Client &user, int client_fd);
		int getClientFdByName(const std::string& nickname);
		void handleNewConnection(std::vector<struct pollfd>& fds);
		void handleClientDisconnection(std::vector<struct pollfd>& fds, size_t i, int bytes_received, const std::string &leaveMsg);
		bool handleClientData(std::vector<struct pollfd>& fds, size_t i);
		void handleClientWrite(std::vector<struct pollfd>& fds, size_t i);
		void handleClientError(std::vector<struct pollfd>& fds, size_t i);
		bool checkBuffer(const std::string& buffer);
		void checkRegist(int client_fd);
		std::string getPass();
		std::string sendMessage(std::string &buff);
		void handleCommand(Client &user, int client_fd);
		void handleNick(int client_fd, const std::string& message);
		void handlePass(int client_fd, const std::string& message);
		void handleUser(int client_fd, const std::string& message);
		void handlePing(int client_fd, const std::string& message);
		void handleJoin(int client_fd, std::string& channel_name, const std::string& pass);
		void handleWho(int client_fd, const std::string& message);
		void handlePrivmsg(int client_fd, const std::string& message);
		void handleRegistration(const std::string& cmd, int client_fd, const std::string& line);
		void handleUserCommand(const std::string& cmd, std::istringstream& cmdStream, int client_fd, const std::string& line);
		void							makeUserList(std::string channel);
		void							broadcastMessageToChannel(const std::string& message, std::string channel);
		void							broadcastMessageToClients(const std::string& message);
		std::vector<Client>::iterator	getClient(int clientFd);
		void							checkCommandPart(std::istringstream &lineStream);
		void							commandPart(std::string &channelName, const std::string &msg);
		bool							LookClientInChannel(std::string channel);
		std::string						getChannelTopic(std::string channel);
		void							changeChannelTopic(const std::string &channel, std::string &newTopic);
		void							checkCommandTopic(std::istringstream &lineStream);
		void							commandTopic(const std::string &channelName, std::string &newTopic);
		void							removeClientsFromChannels(int clientFd, const std::string &msg);
		void							checkCommandJoin(int client_fd, std::istringstream &lineStream);
		void							commandQuit(std::vector<struct pollfd>& fds, size_t i, std::istringstream &msg);
		void							checkCommandBot(std::istringstream &lineStream);
		void							commandBot(std::string &channelName, const std::string &msg);
		bool							LookBotInChannel(std::string channel);
		void							JoinBot(int client_fd, const std::string& channel_name);
		void							PartBot(const std::string &channelName);
		void							PrivmsgBot(const std::string& channel, const std::string& msg);
		std::string						getMsg();
		void handleKick(int client_fd, const std::string& message);
		void handleInvite(int client_fd, const std::string& message);
		void handleMode(int client_fd, const std::string& message);
		bool checkUserExists(const std::string& nickname) const;
		void welcome_messages(int client_fd);
		class ClientFdMatcher
		{
		public:
			ClientFdMatcher(int fd);
			bool operator()(const Client &client) const;
		private:
			int _fd;
		};
};

std::string getLower(const std::string& str);
std::string getFullMsg(std::string &msg, std::istringstream &lineStream, int size);

#endif
