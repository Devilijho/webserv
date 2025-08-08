sources =	main.cpp \
			Requests/server.cpp \
			Requests/Request.cpp \
			RequestHandler/RequestHandler.cpp \
			config/ConfigParser.cpp \
			config/ServerConfig.cpp

cc = c++

cflags = -Wall -Wextra -Werror -std=c++98 

NAME = webserv

all: $(NAME)

$(NAME): $(objects)
	$(cc) $(cflags) -o $(NAME) $(objects)

build/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(cc) $(cflags) -c $< -o $@

clean:
	rm -rf build

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re