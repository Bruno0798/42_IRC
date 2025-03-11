NAME = ircserv

SRC = main.cpp Server.cpp Client.cpp Channel.cpp Commands.cpp helpFunctions.cpp channelControl.cpp utils.cpp Commands/ping.cpp Commands/join.cpp Commands/pass.cpp bonus/Bot.cpp
SRC += Commands/nick.cpp

OBJ = $(SRC:.cpp=.o)

CFLAGS = -Wall -Werror -Wextra
CFLAGS = -std=c++98

all: $(NAME)

$(NAME): $(OBJ)
		c++ -o $(NAME) $(OBJ)

%.o: %.cpp
	c++ $(CFLAGS) -c $< -o $@

go: re
	clear
	./ircserv 6667 ola

clean:
		rm -rf $(OBJ)

fclean: clean
		rm -f $(NAME)

re: fclean all

# Leaks/Valgrind test target
leaks: $(NAME)
ifeq ($(OS), Darwin)
	leaks -atExit -- ./$(NAME) 6667 ola
else
	valgrind --leak-check=full --track-origins=yes ./$(NAME) 6667 ola
endif