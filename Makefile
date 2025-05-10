NAME = server

SRC = main.cpp PollHandler.cpp ServerSocket.cpp CommandParser.cpp Utils.cpp Logger.cpp ClientManager.cpp

FLAGS = -Wall -Wextra -Werror #-fsanitize=address

CPP_STANDARD = -std=c++98

OBJECT = $(SRC:%.cpp=%.o)

all: $(NAME)

$(NAME): $(OBJECT)
	c++ $(OBJECT) $(FLAGS) -o $(NAME)

%.o:%.cpp
	c++ $(CPP_STANDARD) $(FLAGS) -c $<

clean:
	rm -f $(OBJECT)

fclean: clean
	rm -f $(NAME)

re: fclean all