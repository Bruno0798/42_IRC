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
	int _port;
	std::set<std::string> _joinedChannels; // Keep track of joined channels

	bool _registered;
	bool _authenticated;
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
	char *getBuffer() const;
	bool isRegistered() const;

	// Setters
	void setFd(int fd);
	void setIpAddress(const std::string& ip);
	void setNickname(const std::string nickname);
	void setUserName(const std::string username);
	void setRealName(const std::string realname);
	void setPassword(const std::string password);
	void setRegistered(bool registered);

	void authenticate();

	void joinChannel(const std::string& channelName) {
		_joinedChannels.insert(channelName);
	}

	void leaveChannel(const std::string& channelName) {
		_joinedChannels.erase(channelName);
	}

	const std::set<std::string>& getJoinedChannels() const { return _joinedChannels; }

};

#endif
