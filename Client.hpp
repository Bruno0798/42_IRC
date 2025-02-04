#ifndef CLIENT_H
#define CLIENT_H

#include "Irc.hpp"

class Client
{
private:
	int _fd;
	int _port;
	std::string _ipAddress;
	std::string _nickname;
	std::string _username;
	std::string _realname;
	std::vector<std::string> _channels;
	bool _registered;
public:
	Client();
	Client(int fd, const std::string& address, int port);
	~Client();

	//getters
	int getFd() const;
	std::string getNickname() const;
	std::string getUsername() const;
	std::string getRealname() const;
	std::vector<std::string>& getChannels() const;
	bool isRegistered() const;

	//setters
	void setFd(int fd);
	void setIpAddress(const std::string& ip);
	void setNickname(std::string nickname);
	void setUserName(std::string username);
	void setRealName(std::string realname);
	void setRegistered(bool registered);
};

#endif