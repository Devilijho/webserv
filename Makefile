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

NAME = webserv

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(cc) $(cflags) -c $< -o $@

$(NAME): $(objects)
	$(cc) $(cflags) -o $(NAME) $(objects)

all: $(NAME)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean re fclean
