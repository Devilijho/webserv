sources =	main.cpp \
			Requests/server.cpp \
			RequestHandler/RequestHandler.cpp \
			RequestHandler/RequestHandlerExtra.cpp \
			config/ConfigParser.cpp \
			config/ServerConfig.cpp

objects = $(sources:.cpp=.o)

cc = c++

cflags = -Wall -Wextra -Werror -fsanitize=address -std=c++98
# -I.. -I../config -I../CGI
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
