#ifndef SERVER_H
#define SERVER_H

# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <cstring>
# include <cstdlib>
# include <iostream>
# include <vector>
# include <map>
# include <poll.h>
# include <unistd.h>
# include <sstream>
# include <string>
# include <cerrno>
# include <ctime>

class Server
{
private:
	struct addrinfo _address;
	struct addrinfo *_servinfo;
	int _port;
	int _fd;
	std::string _password;
public:
	Server();
	Server(int _port, std::string& password);
	~Server();

	void setAddress();
	bool fillServerInfo(char *port);
	bool setupServer();
};

#endif