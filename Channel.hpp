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
		bool _priv;                                        // -|
		std::vector<int> _allowedClients; // Para ser usado pelos canais privados 


	public:
		Channel() {} // Default constructor
		Channel(const std::string& name) : _name(name) , _priv(false){}

		void 								setName(std::string name);
		void 								setTopic(std::string topic);
		void								setPrivate();
		void								setAllowedClient(int clientFd);
		
		const std::string& 								getName();
		const std::string& 								getTopic();
		const std::map<int, std::vector<std::string> >& getClients() const;
		bool											getPrivate();
		const std::vector<int>&							getAllowedClients() const;

		void											removeClient(int client_fd);
		void											addClient(int client_fd);
		


};

#endif // CHANNEL_HPP