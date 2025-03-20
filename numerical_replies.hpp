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
# define ERR_BADCHANMASK(channel) (":localhost 476 " + (channel) + " :Bad Channel Mask\r\n")
# define RPL_TOPIC(nickname, channel, topic) (":localhost 332 " + (nickname) + " " + (channel) + " " + (topic) + "\r\n")

//KICK
# define ERR_USERNOTINCHANNEL(nickname, nick, channel) (":localhost 441 " + (nickname) + " " + (nick) + " " + (channel) + " :They aren't on that channel")
# define ERR_NOSUCHNICK(nickname, nick) (":localhost 401 " + (nickname) + " " + (nick) + " :No such nick/channel\r\n")

//MODE

# define RPL_CHANNELMODEIS(nickname, channel, mode) (":localhost 324 " + (nickname) + " " + (channel) + " " + (mode) + "\r\n")
# define ERR_UMODEUNKNOWNFLAG(nickname) (":localhost 501 " + (nickname) + " :Unknown MODE flag\r\n")
# define ERR_NOSUCHCHANNEL(nickname, channel) (":localhost 403 " + (nickname) + " " + (channel) + " :No such channel\r\n")


//PRIVMSG
# define ERR_NORECIPIENT(nickname, command) (":localhost 411 " + (nickname) + " :No recipient given " + (command) + "\r\n")
# define ERR_NOTEXTTOSEND(nickname) (":localhost 412 " + (nickname) + " :No text to send\r\n")
# define ERR_CANNOTSENDTOCHAN(nickname, channel) (":localhost 404 " + (nickname) + " " + (channel) + " :Cannot send to channel\r\n")

# define ERR_NEEDMOREPARAMS(nickname, command) (":localhost 461 " + (nickname) + " " + (command) + " :Not enough parameters\r\n")
# define ERR_ALREADYREGISTERED(nickname) (":localhost 462 " + (nickname) + " :You may not reregister\r\n")
# define ERR_NOTONCHANNEL(nickname, channel) (":localhost 442 " + (nickname) + " " + (channel) + " :You're not on that channel\r\n")
# define ERR_CHANOPRIVSNEEDED(nickname, channel) (":localhost 482 " + (nickname) + " " + (channel) + " :You're not channel operator\r\n")
# define ERR_USERONCHANNEL(nickname, nick, channel) (":localhost 443 " + (nickname) + " " + (nick) + " " + (channel) + " :is already on channel\r\n")
# define ERR_UNKNOWNCOMMAND(nickname, command) (":localhost 421 " + (nickname) + " " + (command) + " :Unknown command\r\n")




#endif //NUMERICAL_REPLIES_HPP