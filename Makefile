NAME = server

SRC = main.cpp PollHandler.cpp ServerSocket.cpp

FLAGS = -Wall -Wextra -Werror

CPP_STANDARD = -std=c++98

OBJECT = $(SRC:%.cpp=%.o)

all: $(NAME)

$(NAME): $(OBJECT)
	c++ $(OBJECT) -o $(NAME)

%.o:%.cpp
	c++ $(CPP_STANDARD) -c $<

clean:
	rm -f $(OBJECT)

fclean: clean
	rm -f $(NAME)

re: fclean all