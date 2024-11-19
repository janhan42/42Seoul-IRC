# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: janhan <janhan@student.42seoul.kr>         +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/04/24 15:49:02 by janhan            #+#    #+#              #
#    Updated: 2024/11/19 08:28:18 by janhan           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME			=		ircserv

CXX				=		c++
CXXFLAGS		=		-Wall -Wextra -Werror -std=c++98

SRCS_DIR	= ./srcs/
SRCS		=	User.cpp					\
				Server.cpp					\
				main.cpp					\
				Command.cpp					\
				Channel.cpp					\
				Bot.cpp
OBJS_SRCS	=	$(addprefix $(SRCS_DIR), $(SRCS:.cpp=.o))

UTILS_DIR	=	./srcs/Utils/
UTILS		=	IsSpecial.cpp				\
				Split.cpp
OBJS_UTILS	=	$(addprefix $(UTILS_DIR), $(UTILS:.cpp=.o))

ERROR_DIR	=	./srcs/ErrorUtils/
ERRORS		=	ResponseManager.cpp
OBJS_ERROR	=	$(addprefix $(ERROR_DIR), $(ERRORS:.cpp=.o))

CMDS_DIR	=	./srcs/Commands/
CMDS		=	Invite.cpp					\
				Join.cpp					\
				Kick.cpp					\
				Mode.cpp					\
				Nick.cpp					\
				Part.cpp					\
				Pass.cpp					\
				Ping.cpp					\
				Privmsg.cpp					\
				Quit.cpp					\
				Topic.cpp					\
				User.cpp
OBJS_CMDS	=	$(addprefix $(CMDS_DIR), $(CMDS:.cpp=.o))
OBJS		=	$(OBJS_SRCS) $(OBJS_UTILS) $(OBJS_CMDS) $(OBJS_ERROR)

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

