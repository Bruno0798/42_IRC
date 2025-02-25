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
	std::map<int, std::vector<std::string> >::iterator it = _clients.find(client_fd);
    
    if (it != _clients.end())
    {
        _clients.erase(it);
        std::cout << "Client " << client_fd << " removed from channel " << _name << std::endl;
    }
}


bool	Channel::getPrivate()
{
	return _priv;
}

const std::vector<int>&		Channel::getAllowedClients() const
{
	return _allowedClients;
}

void	Channel::setPrivate()
{
	_priv = true;
}

void	Channel::setAllowedClient(int clientFd)
{
	_allowedClients.push_back(clientFd);
}
