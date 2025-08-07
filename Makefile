sources = main.cpp CGI/CGIHandler.cpp

CXX = c++

CXXFLAGS = -Wall -Wextra -Werror -std=c++98

NAME = identifier

SRCS = main.cpp server.cpp

all: $(NAME)

$(NAME):
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(NAME)

clean:
	rm -f $(NAME)

fclean: clean

re: fclean all

.PHONY: all clean re fclean
