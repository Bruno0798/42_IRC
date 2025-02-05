#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <string>

class Client
{
private:
	int _fd;
	std::string _ipAddress;
	int _port;
	std::string _nickname;
	std::string _username;
	std::string _realname;
	bool _registered;

public:
	Client();
	Client(int fd); // New constructor
	Client(int fd, const std::string& address, int port);
	~Client();

	// Getters
	int getFd() const;
	std::string getNickname() const;
	std::string getUsername() const;
	std::string getRealname() const;
	bool isRegistered() const;

	// Setters
	void setFd(int fd);
	void setIpAddress(const std::string& ip);
	void setNickname(const std::string nickname);
	void setUserName(const std::string username);
	void setRealName(const std::string realname);
	void setRegistered(bool registered);
};

#endif
