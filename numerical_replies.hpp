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
# define ERR_PASSWDMISMATCH(nickname) (":localhost 464 " + (nickname) + " :Password incorrect\r\n")
# define ERR_NOTAUTHENTICATED(nickname) (":localhost 451 " + (nickname) + " :You have not authenticated\r\n")
# define ERR_NOTREGISTERED(nickname) ("localhost 451 " + (nickname) + " :You have not registered\r\n")

//NICK
# define ERR_NONICKNAMEGIVEN(nickname) (":localhost 431 " + (nickname) + " :No nickname given\r\n")
# define ERR_ERRONEUSNICKNAME(nickname, nick) (":localhost 432 " + (nickname) + " " + (nick) + " :Erroneus nickname\r\n")
# define ERR_NICKNAMEINUSE(nickname, nick) (":localhost 433 " + (nickname) + " " + (nick) + " :Nickname is already in use\r\n")

//JOIN
# define RPL_NOTOPIC(nickname, channel) ("localhost 331 " + (nickname) + " " + (channel) + " :No topic is set\r\n")
# define ERR_CHANNELISFULL(nickname, channel) (":localhost 471 " + (nickname) + " " + (channel) + " :Cannot join channel (+l)\r\n")
# define ERR_INVITEONLYCHAN(nickname, channel) (":localhost 473 " + (nickname) + " " + (channel) + " :Cannot join channel (+i)\r\n")
# define ERR_BADCHANNELKEY(nickname, channel) (":localhost 475 " + (nickname) + " " + (channel) + " Cannot join channel (+k)\r\n")




# define ERR_NEEDMOREPARAMS(nickname, command) (":localhost 461 " + (nickname) + " " + (command) + " :Not enough parameters\r\n")
# define ERR_ALREADYREGISTERED(nickname) (":localhost 462 " + (nickname) + " :You may not reregister\r\n")




#endif //NUMERICAL_REPLIES_HPP