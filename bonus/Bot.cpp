#include "Bot.hpp"



// Constructor
Bot::Bot() : _name("Step Bro")
{
    _responses["hello"] = "Hello! How can I assist you?";
    _responses["help"] = "I am a bot! Try commands like 'hello' or 'bye'.";
    _responses["bye"] = "Goodbye! Have a great day!";
}

Bot::~Bot()
{
}

std::string Bot::getName() const
{
    return _name;
}

void Bot::setName(std::string& name) 
{
    _name = name;
}

std::string Bot::processMessage(const std::string& message) const
{
    std::map<std::string, std::string>::const_iterator it = _responses.find(message);
    if (it != _responses.end())
        return it->second;
    return "I don't understand that command.";
}

void Bot::addResponse(const std::string& trigger, const std::string& response)
{
    _responses[trigger] = response;
}
