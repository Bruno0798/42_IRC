#include "Channel.hpp"
#include "Client.hpp"



void Channel::addClient(int client_fd)
{
	this->_clients.insert(std::make_pair(client_fd, std::vector<std::string>()));
}

const std::string& Channel::getName()
{
	return _name;
}

void Channel::setName(std::string name)
{
	_name = name;
}

void Channel::setTopic(std::string topic)
{
	_topic = topic;
}

const std::string& Channel::getTopic()
{

	return _topic;

}

const std::map<int, std::vector<std::string> >& Channel::getClients() const
{
	return _clients;
}

void	Channel::removeClient(int client_fd)
{	
	
}
