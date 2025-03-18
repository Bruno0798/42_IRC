#ifndef NUMERICAL_REPLIES_HPP
#define NUMERICAL_REPLIES_HPP

# define SOURCE(nickname, username) (":" + nickname + "!" + username + "@localhost ")

//WELCOME MESSAGES
# define RPL_WELCOME(nickname, username) (":localhost 001 " + (nickname) + " :Welcome to the IRC, " + (nickname) + "\r\n")
# define RPL_YOURHOST(nickname) (":localhost 002 " + (nickname) + " :Your host is localhost\r\n")
# define RPL_CREATED(nickname, time) (":localhost 003 " + (nickname) + " :This server was created " + (time) + "\r\n")
# define RPL_MYINFO(nickname) (":localhost 004 " + (nickname) + " :localhost v1.0 o iklt\r\n")
# define RPL_ISUPPORT(nickname) (":localhost 005 " + (nickname) + " :CHANMODES=o,i,k,l,t\r\n")

//PASS
#define ERR_ALREADYREGISTERED(nickname) (":localhost 462 " + (nickname) + " :You may not reregister\r\n")
#define ERR_PASSWDMISMATCH(nickname) (":localhost 464 " + (nickname) + " :Password incorrect\r\n")

//NICK
#define ERR_NONICKNAMEGIVEN(nickname) (":localhost 431" + (nickname) + " :No nickname given\r\n")
#define ERR_ERRONEUSNICKNAME(nickname, nick) (":localhost 432" + (nickname) + " " + (nick) + " :Erroneus nickname\r\n")





# define ERR_NEEDMOREPARAMS(nickname, command) (":localhost 461 " + (nickname) + " " + (command) + " :Not enough parameters\r\n")





#endif //NUMERICAL_REPLIES_HPP