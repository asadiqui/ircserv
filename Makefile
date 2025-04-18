NAME = server

SRC = main.cpp ServerSocket.cpp

FLAGS = -Wall -Wextra -Werror

CPP_STANDARD = -std=c++98

OBJECT = $(SRC:%.cpp=%.o)

all: $(NAME)

$(NAME): $(OBJECT)
	c++ $(OBJECT) -o $(NAME)

clean:
	rm -f $(OBJECT)

fclean: clean
	rm -f $(NAME)

re: fclean all