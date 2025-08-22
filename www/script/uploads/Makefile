SRC	=	ft_putchar.c \
		ft_putstr.c \
		ft_strcmp.c \
		ft_strlen.c \
		ft_swap.c

SRCS	=	${addprefix ${PRE}, ${SRC}}

OBJS	=	$(SRCS:.c=.o)

PRE	=	./srcs/

HEAD	=	./includes/

CC	=	gcc

CFLAGS	=	-Wall -Wextra -Werror

NAME	=	libft.a

all	:	$(NAME)

.c.o	:
		${CC} ${CFLAGS} -c -I ${HEAD} $< -o ${<:.c=.o}
#pour creer la library

$(NAME)	:	$(OBJS)
		ar -rc $@ $^

clean	:	rm -f $(OBJS)

fclean	:	clean
		rm -f $(NAME)

re	:	fclean all

.PHONY	:	all clean fclean re
