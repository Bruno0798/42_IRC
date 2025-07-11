#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <string>
#include <set>

class Client
{
private:
	int _fd;
	std::string _ipAddress;
	std::string _nickname;
	std::string _username;
	std::string _realname;
	std::string _password;
	std::string _buffer;
	std::set<std::string> _joinedChannels; // Keep track of joined channels

	bool _registered;
	bool _authenticated;
public:

	Client();
	Client(int fd); // New constructor
	Client(int fd, const std::string& address);
	~Client();

	// Getters
	int getFd() const;
	std::string getNickname() const;
	std::string getUsername() const;
	std::string getRealname() const;
	std::string getPassword() const;
	std::string getBuffer() const;
	std::string &getAccumulatedBuffer() { return _buffer; }
	bool isRegistered() const;
	bool isAuth() const;

	// Setters
	void setFd(int fd);
	void setIpAddress(const std::string& ip);
	void setNickname(const std::string nickname);
	void setUserName(const std::string username);
	void setRealName(const std::string realname);
	void setPassword(const std::string password);
	void setBuffer(char *buf);
	void delete_buffer();
	void setRegistered(bool registered);
	void setAuth(bool auth);

	bool authenticate();

	void joinChannel(const std::string& channelName);

};

#endif
