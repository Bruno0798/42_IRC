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

const std::vector<int>&		Channel::getAllowedClients() const
{
	return _allowedClients;
}

void	Channel::setAllowedClient(int clientFd)
{
	_allowedClients.push_back(clientFd);
}

void Channel::setPass(std::string pass)
{
	_pass = pass;
}

void Channel::removePass()
{
	_pass = "";
}

void Channel::revokePermissions(int client_fd)
{
	std::set<int>::iterator op_it = std::find(_operators.begin(), _operators.end(), client_fd);
	if (op_it != _operators.end())
		_operators.erase(op_it);

	std::vector<int>::iterator invite_it = std::find(_allowedClients.begin(), _allowedClients.end(), client_fd);
	if (invite_it != _allowedClients.end())
		_allowedClients.erase(invite_it);
}