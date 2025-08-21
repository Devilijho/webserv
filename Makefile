sources =	main.cpp \
			Requests/server.cpp \
			Requests/startsetup.cpp \
			Requests/handleReadEvent.cpp \
			Requests/handleWriteEvent.cpp \
			RequestHandler/RequestHandler.cpp \
			RequestHandler/RequestHandlerExtra.cpp \
			config/ConfigParser.cpp \
			config/ServerConfig.cpp

OBJ_DIR = build

objects = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(sources))

cc = c++

cflags = -Wall -Wextra -Werror -fsanitize=address -std=c++98
# -I.. -I../config -I../CGI

NAME = webserv

$(NAME): $(objects)
	$(cc) $(cflags) -o $(NAME) $(objects)

all: $(NAME)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(cc) $(cflags) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean re fclean
