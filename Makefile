NAME = ircserv

SRC = main.cpp Server.cpp Client.cpp Channel.cpp Commands.cpp helpFunctions.cpp
OBJ = $(SRC:.cpp=.o)

CFLAGS = -Wall -Werror -Wextra
CFLAGS = -std=c++98

all: $(NAME)

$(NAME): $(OBJ)
		c++ -o $(NAME) $(OBJ)

%.o: %.cpp
	c++ $(CFLAGS) -c $< -o $@

go:
	make re
	./ircserv 6667 ola

clean:
		rm -rf $(OBJ)

fclean: clean
		rm -f $(NAME)

re: fclean all

# Leaks/Valgrind test target
leaks: $(NAME)
ifeq ($(OS), Darwin)
	leaks -atExit -- ./$(NAME)
else
	valgrind --leak-check=full --track-origins=yes ./$(NAME)
endif