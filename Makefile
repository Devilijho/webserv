sources =	main.cpp \
			server.cpp \
			RequestHandler/CGIHandler.cpp \
			config/ConfigParser.cpp \
			config/ServerConfig.cpp

objects = $(sources:.cpp=.o)

cc = c++

cflags = -Wall -Wextra -Werror -std=c++98

NAME = webserv

$(NAME): $(objects)
	$(cc) $(cflags) -o $(NAME) $(objects)

all: $(NAME)

%.o: %.cpp
	$(cc) $(cflags) -c $< -o $@

clean:
	rm -f ${objects}

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean re fclean
