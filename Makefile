sources =	main.cpp \
			Requests/server.cpp \
			Requests/utils.cpp \
			Requests/startsetup.cpp \
			Requests/handleReadEvent.cpp \
			Requests/utilsReadEvent.cpp \
			Requests/handleWriteEvent.cpp \
			RequestHandler/RequestHandler.cpp \
			RequestHandler/RequestHandlerExtra.cpp \
			config/ConfigParser_1.cpp \
			config/ConfigParser_2.cpp \
			config/ConfigParser_3.cpp \
			config/ConfigParser_4.cpp \
			config/ConfigParser_5.cpp \
			config/ConfigParser_6.cpp \
			config/ServerConfig2.cpp

OBJ_DIR = build

objects = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(sources))

cc = c++
cflags = -Wall -Wextra -Werror -std=c++98 #-fsanitize=address


NAME = webserv

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(cc) $(cflags) -c $< -o $@

$(NAME): $(objects)
	$(cc) $(cflags) -o $(NAME) $(objects)

all: $(NAME)

%.o: %.cpp
	$(cc) $(cflags) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean re fclean
