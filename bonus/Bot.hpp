#ifndef BOT_HPP
#define BOT_HPP

#include <iostream>
#include <string>
#include <map>
#include "../Server.hpp"

class Bot
{
    private:
        std::string _name;
        int         _botFd;
        std::map<std::string, std::string> _responses;

    public:

        Bot();
        ~Bot();


        std::string getName() const;
        void        setName(std::string& name);
        std::string processMessage(const std::string& message) const;
        void        addResponse(const std::string& trigger, const std::string& response);
};

#endif
