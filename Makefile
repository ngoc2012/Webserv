# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: nbechon <nbechon@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2022/09/28 10:56:09 by minh-ngu          #+#    #+#              #
#    Updated: 2024/02/12 10:29:41 by ngoc             ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

FILES = main Host Address Server Worker Request RequestHeader Response Header Configuration Location Cgi Listing Sessions ft/split_string ft/is_digit ft/itos ft/match_wildcard ft/file_extension ft/atoi_base ft/itoa_base ft/str_replace ft/to_upper ft/trim_string ft/timestamp
SRCS = $(addsuffix .cpp, $(addprefix srcs/, $(FILES)))
INCS = $(wildcard includes/*.hpp)
OBJS = ${SRCS:.cpp=.o}
MANDA = webserv
CC = c++
FLAGS = -Wall -Wextra -Werror -std=c++98
all:	$(MANDA)
.cpp.o:
	$(CC) $(FLAGS) -g -c $< -o ${<:.cpp=.o} -I./includes
$(MANDA): $(SRCS) $(OBJS) $(INCS)
	$(CC) $(FLAGS) $(OBJS) -o $(MANDA)
test:
	clear && make re && make clean && ./webserv
test0:
	clear && make re && make clean && valgrind --track-origins=yes --track-fds=yes --leak-check=full --show-leak-kinds=all ./webserv
gits:
	git add Makefile
	git add *.cpp
	git add *.hpp
	git commit -m "all"
	git push
others:
	./ubuntu_tester http://0.0.0.0:8012
	curl -i -X GET http://127.0.2.1:4242
	curl -i -X POST http://127.0.2.1:4242/directory/youpi.bla
	curl -i -X POST http://0.0.0.0:8012/directory/youpi.bla

M:=
gitd:
	make fclean
	git add Makefile
	git add *.cpp
	git add *.hpp
	git add .conf
	git add www
	git add tester
	git add webserv-master
	#git commit -m "all"
	#git add -A -- :!*.o :!*.swp :!*.env
	git commit -m "$(M)"
	git push
clean:
	rm -f $(OBJS)
fclean: clean
	rm -f $(MANDA)
re: fclean all
.PHONY: all clean fclean re test
