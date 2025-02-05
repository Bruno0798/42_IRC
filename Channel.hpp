// In Channel.hpp
#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>

class Channel
{
public:
	Channel() {} // Default constructor
	Channel(const std::string& name) : _name(name) {}

	void addClient(int client_fd) { _clients.push_back(client_fd); }
	const std::string& getName() const { return _name; }

private:
	std::string _name;
	std::vector<int> _clients;
};

#endif // CHANNEL_HPP