NAME = ircserv
NAME_bonus = ircserv_bonus

SRC = main.cpp Server.cpp Client.cpp Channel.cpp Commands.cpp helpFunctions.cpp part.cpp utils.cpp Commands/ping.cpp Commands/join.cpp Commands/pass.cpp
SRC += Commands/nick.cpp Commands/privmsg.cpp Commands/topic.cpp Commands/user.cpp Commands/quit.cpp 

SRC_bonus = main.cpp Server.cpp Client.cpp Channel.cpp bonus/Commands_bonus.cpp helpFunctions.cpp bonus/part_bonus.cpp utils.cpp Commands/ping.cpp Commands/join.cpp Commands/pass.cpp
SRC_bonus += Commands/nick.cpp Commands/privmsg.cpp Commands/topic.cpp Commands/user.cpp Commands/quit.cpp bonus/Bot_bonus.cpp

OBJ = $(SRC:.cpp=.o)

OBJ_bonus = $(SRC_bonus:.cpp=.o)

CFLAGS = -Wall -Werror -Wextra
CFLAGS = -std=c++98

all: $(NAME)

$(NAME): $(OBJ)
		c++ -o $(NAME) $(OBJ)

%.o: %.cpp
	c++ $(CFLAGS) -c $< -o $@

bonus: $(NAME_bonus)

$(NAME_bonus): $(OBJ_bonus)
		c++ -o $(NAME_bonus) $(OBJ_bonus)
%.o: %.cpp
	c++ $(CFLAGS) -c $< -o $@

go: all
	clear
	./ircserv 6667 ola

bonusgo: bonus
		clear
		./ircserv_bonus 6667 ola

clean:
		rm -rf $(OBJ)
		rm -rf $(OBJ_bonus)

fclean: clean
		rm -f $(NAME)
		rm -f $(NAME_bonus)

re: fclean all

# Leaks/Valgrind test target
leaks: re
ifeq ($(OS), Darwin)
	leaks -atExit -- ./$(NAME) 6667 ola
else
	valgrind --leak-check=full --track-origins=yes ./$(NAME) 6667 ola
endif

lekas: re
ifeq ($(OS), Darwin)
	leaks -atExit -- ./$(NAME) 6667 ola
else
	valgrind --leak-check=full --track-origins=yes ./$(NAME) 6667 ola
endif