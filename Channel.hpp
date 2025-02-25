// In Channel.hpp
#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "Client.hpp"
#include "Irc.hpp"


class Channel
{
	private:
		
		std::string _name;
		std::map<int, std::vector<std::string> > _clients; // Os clientes e as suas permicoes
		std::string _topic;
		std::string _pass;                                     
		std::vector<int> _allowedClients; // Para ser usado pelos canais privados 

		std::set<int> _operators;
		bool _inviteOnly;
		bool _topicRestricted;
		size_t _userLimit;
		bool _hasUserLimit;

	public:
		Channel() : _inviteOnly(false), _topicRestricted(false), 
                   _hasUserLimit(false), _userLimit(0) {}
		Channel(const std::string& name) : _name(name), _inviteOnly(false),
                _topicRestricted(false), _hasUserLimit(false), _userLimit(0) {}

		void 								setName(std::string name);
		void 								setTopic(std::string topic);
		void								setAllowedClient(int clientFd);
		void								setPass(std::string pass);
		void								removePass();
		
		const std::string& 								getName();
		const std::string& 								getTopic();
		const std::map<int, std::vector<std::string> >& getClients() const;
		const std::vector<int>&							getAllowedClients() const;

		void											removeClient(int client_fd);
		void											addClient(int client_fd);
		
		 bool isOperator(int client_fd) const 
        { 
            return _operators.find(client_fd) != _operators.end(); 
        }

        void addOperator(int client_fd) 
        { 
            _operators.insert(client_fd); 
        }

        void removeOperator(int client_fd) 
        { 
            _operators.erase(client_fd); 
        }

        // Mode +i (invite-only)
        bool isInviteOnly() const { return _inviteOnly; }
        void setInviteOnly(bool value) { _inviteOnly = value; }

        // Mode +t (topic restriction)
        bool isTopicRestricted() const { return _topicRestricted; }
        void setTopicRestricted(bool value) { _topicRestricted = value; }

        // Mode +l (user limit)
        bool hasUserLimit() const { return _hasUserLimit; }
        size_t getUserLimit() const { return _userLimit; }
        void setUserLimit(size_t limit) 
        { 
            _userLimit = limit;
            _hasUserLimit = true;
        }
        void removeUserLimit() 
        { 
            _hasUserLimit = false;
            _userLimit = 0;
        }

        bool canJoin(int client_fd) const 
        {
            if (_hasUserLimit && _clients.size() >= _userLimit)
                return false;
            if (_inviteOnly && std::find(_allowedClients.begin(), _allowedClients.end(), client_fd) == _allowedClients.end())
                return false;
            return true;
        }

        bool hasClient(int client_fd) const
        {
            return _clients.find(client_fd) != _clients.end();
        }

        std::string getModeString() const 
        {
            std::string modes = "+";
            if (_inviteOnly) modes += "i";
            if (_topicRestricted) modes += "t";
            if (_hasUserLimit) modes += "l";
            return modes;
        }
};

#endif // CHANNEL_HPP