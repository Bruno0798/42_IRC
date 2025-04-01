// In Channel.hpp
#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "Client.hpp"
#include "Irc.hpp"


class Channel
{
	private:
		
		std::string _name;
		std::map<int, std::vector<std::string> > _clients;
		std::string _topic;
		std::string _pass;                                     
		std::vector<int> _allowedClients;

		std::set<int> _operators;
		bool _inviteOnly;
		bool _topicRestricted;
		size_t _userLimit;
		bool _hasUserLimit;

	public:
		Channel();
		Channel(const std::string& name);

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
		void								revokePermissions(int client_fd);

		
		bool 								isOperator(int client_fd) const;

		// Mode +o (operator)
        void 								addOperator(int client_fd);
        void 								removeOperator(int client_fd);
        
		bool								isPasswordProtected() const;
		const std::string& 					getPass() const;

        // Mode +i (invite-only)
        bool 								isInviteOnly() const;
        void 								setInviteOnly(bool value);

        // Mode +t (topic restriction)
        bool 								isTopicRestricted() const;
        void 								setTopicRestricted(bool value);

        // Mode +l (user limit)
        bool 								hasUserLimit() const;
        size_t 								getUserLimit() const;
        void 								setUserLimit(size_t limit);
        void 								removeUserLimit();

        int 								canJoin(int client_fd, const std::string &pass) const;
        bool 								hasClient(int client_fd) const;
        std::string 						getModeString() const;
		bool 								isUserInChannel(int client_fd) const;
};

#endif // CHANNEL_HPP
