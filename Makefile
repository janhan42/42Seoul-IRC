# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: janhan <janhan@student.42seoul.kr>         +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/04/24 15:49:02 by janhan            #+#    #+#              #
#    Updated: 2024/10/13 10:35:02 by janhan           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME			=		irc-serv

CXX				=		c++
CXXFLAGS		=		-Wall -Wextra -Werror -std=c++98

SRCS		=	main.cpp				\
				src/Irc/User.cpp		\
				src/server/Server.cpp	\
				src/utils/IsInt.cpp
OBJS		=	$(SRCS:.cpp=.o)

all : $(NAME)

$(NAME) : $(OBJS)
	@echo $(CURSIVE)$(YELLOW) "      - Making $(NAME) -" $(NONE)
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $@
	@echo $(CURSIVE)$(YELLOW) "        - Compiling $(NAME) -" $(NONE)

%.o : %.cpp
	@echo $(CURSIVE)$(YELLOW) "      - Making object files -" $(NONE)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean :
	@rm -fr $(OBJS)
	@echo $(CURSIVE)$(BLUE) "     - clean OBJ files -" $(NONE)

fclean : clean
	@rm -fr $(NAME)
	@echo $(CURSIVE)$(PURPLE)"      - clean $(NAME) file -"$(NONE)

re	:
	@make fclean
	@make all

.PHONY: all make clean fclean bonus re

NONE='\033[0m'
GREEN='\033[32m'
YELLOW='\033[33m'
GRAY='\033[2;37m'
CURSIVE='\033[3m'
PURPLE='\033[35m'
BLUE='\033[34m'
DELETELINE='\033[K;
