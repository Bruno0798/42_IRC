#include "Channel.hpp"
#include "Client.hpp"


Channel::Channel() :_inviteOnly(false), _topicRestricted(false), _userLimit(0), _hasUserLimit(false)
{

}

Channel::Channel(const std::string &name) : _name(name), _inviteOnly(false),
_topicRestricted(false), _userLimit(0), _hasUserLimit(false)
{

}

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

bool Channel::isOperator(int client_fd) const
{ 
		return _operators.find(client_fd) != _operators.end(); 
}

void Channel::addOperator(int client_fd)
{ 
	_operators.insert(client_fd); 
}

void Channel::removeOperator(int client_fd)
{ 
	_operators.erase(client_fd); 
}

bool Channel::isPasswordProtected() const
{
	return !_pass.empty();
}

const std::string& Channel::getPass() const
{
	return _pass;
}

bool Channel::isInviteOnly() const
{ 
	return _inviteOnly; 
}

void Channel::setInviteOnly(bool value)
{ 
	_inviteOnly = value; 
}

bool Channel::isTopicRestricted() const
{ 
	return _topicRestricted; 
}

void Channel::setTopicRestricted(bool value)
{ 
	_topicRestricted = value;
}

bool Channel::hasUserLimit() const
{ 
	return _hasUserLimit;
}

size_t Channel::getUserLimit() const
{ 
	return _userLimit;
}

void Channel::setUserLimit(size_t limit)
{ 
	_userLimit = limit;
	_hasUserLimit = true;
}

void Channel::removeUserLimit()
{ 
	_hasUserLimit = false;
	_userLimit = 0;
}

int Channel::canJoin(int client_fd, const std::string &pass) const
{
	if (_hasUserLimit && _clients.size() >= _userLimit)
		return 471; // ERR_CHANNELISFULL
	if (_inviteOnly && std::find(_allowedClients.begin(), _allowedClients.end(), client_fd) == _allowedClients.end())
		return 473;// ERR_INVITEONLYCHAN
	if (!_pass.empty() && _pass != pass && std::find(_allowedClients.begin(), _allowedClients.end(), client_fd) == _allowedClients.end())
		return 475; // ERR_BADCHANNELKEY
	return 0;
}

bool Channel::hasClient(int client_fd) const
{
	return _clients.find(client_fd) != _clients.end();
}

std::string Channel::getModeString() const
{
	std::string modes = "+";
	if (_inviteOnly) modes += "i";
	if (_topicRestricted) modes += "t";
	if (_hasUserLimit) modes += "l";
	if (_pass.size() > 0) modes += "k";
	return modes;
}


bool Channel::isUserInChannel(int client_fd) const
{
	std::map<int, std::vector<std::string> >::const_iterator it;
	for (it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->first == client_fd)
		{
			return true;
		}
	}
	return false;
}