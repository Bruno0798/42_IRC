NAME = ircserv

SRC = main.cpp Client.cpp Server.cpp Channel.cpp
OBJ = $(SRC:.cpp=.o)

CFLAGS = -Wall -Werror -Wextra -std=c++98

all: $(NAME)

$(NAME): $(OBJ)
		c++ -o $(NAME) $(OBJ)

%.o: %.cpp
	c++ $(CFLAGS) -c $< -o $@

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