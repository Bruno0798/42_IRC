#ifndef NUMERICAL_REPLIES_HPP
#define NUMERICAL_REPLIES_HPP

# define SOURCE(nickname, username) (":" + nickname + "!" + username + "@localhost ")

//WELCOME MESSAGES
# define RPL_WELCOME(nickname, username) (":localhost 001 " + nickname + " :Welcome to the IRC, " + nickname + "\r\n")
# define RPL_YOURHOST(nickname, username) (":localhost 002 " + nickname + " :Your host is localhost\r\n")
# define RPL_CREATED(nickname, username, time) (":localhost 003 " + nickname + " :This server was created " + time + "\r\n")
# define RPL_MYINFO(nickname, username) (":localhost 004 " + nickname + " :localhost v1.0 o iklt \r\n")
# define RPL_ISUPPORT(nickname, username) (":localhost 005 " + nickname + " :CHANMODES=o,i,k,l,t \r\n")

# define ERR_UNKNOWNCOMMAND()



#endif //NUMERICAL_REPLIES_HPP