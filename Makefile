# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: nbechon <nbechon@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2022/09/28 10:56:09 by minh-ngu          #+#    #+#              #
#    Updated: 2024/03/24 21:18:25 by lbastian         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

FILES = main Host Worker Address Server Request Response Header Configuration Location Cgi Listing Sessions ft/split_string ft/is_digit ft/itos ft/match_wildcard ft/file_extension ft/atoi_base ft/itoa_base ft/str_replace ft/to_upper ft/trim_string ft/timestamp ft/print_loading_bar ft/print_size ft/strdup ft/convertisseur
SRCS = $(addsuffix .cpp, $(addprefix srcs/, $(FILES)))
INCS = $(wildcard includes/*.hpp)
OBJS = ${SRCS:.cpp=.o}
MANDA = webserv
CC = c++
FLAGS = -Wall -Wextra -Werror -std=c++98 -pthread -D DELAY=50 -D RUPTURE=0
all:	$(MANDA)
.cpp.o:
	$(CC) $(FLAGS) -g -c $< -o ${<:.cpp=.o} -I./includes
$(MANDA): $(SRCS) $(OBJS) $(INCS)
	$(CC) $(FLAGS) $(OBJS) -o $(MANDA)
clean:
	rm -f $(OBJS)
fclean: clean
	rm -f $(MANDA)
	rm -f logs
re: fclean all

#ANSI escape codes
RESET := \033[0m
RED := \033[0;31m
GREEN := \033[0;32m
YELLOW := \033[0;33m
BLUE := \033[0;34m
MAGENTA := \033[0;35m
CYAN := \033[0;36m
WHITE := \033[0;37m

BOLD := \033[1m
ITALIC := \033[3m
UNDERLINE := \033[4m

B_RED := \033[1;31m
B_GREEN := \033[1;32m
B_YELLOW := \033[1;33m
B_BLUE := \033[1;34m
B_MAGENTA := \033[1;35m
B_CYAN := \033[1;36m
B_WHITE := \033[1;37m

subject2:
	-make
	@echo "===== Look two servers with different port"
	@echo "Check => 127.0.0.1:4141"
	@echo "Check => 127.0.0.1:5050"
	@read -p "Press enter to continue..." continue
#PS : j'ai copier coller 127.0.0.1:4141 et j'ai juste changer le port donc on arrive sur le meme site a voir si ca derange pas dans le .conf
	@echo "===== Setup the server_names or not."
	@echo "** Without server name"
#VRAIMENT PAS SUR --
	-curl -i -X GET http://127.0.3.1:8080
	@read -p "Press enter to continue..." continue
	@echo "** With server name"
	-curl -i -H "Host: loic.com" http://127.0.3.1:8080
	@echo "****************************************************************************"
	-curl -i -H "Host: nathan.fr" http://127.0.3.1:8080
	@read -p "Press enter to continue..." continue
#VRAIMENT PAS SUR
	@echo "===== Default error page"
	-curl -i http://127.0.0.1:4141/toto
	@echo "** Change 404 file (in error_pages)"
	@read -p "Press enter to continue..." continue
	-curl -i http://127.0.0.1:4141/toto
	@echo "===== Limit client body size."
	@read -p "Press enter to continue..." continue
	-curl -i -X POST -H "Transfer-Encoding: chunked" --data-raw "$$(dd if=/dev/urandom bs=120 count=1 status=none | base64 | tr -d '\n' | head -c 120)" http://127.0.2.1:4242/post_body
#=>	@echo "Montrer une routes in server" ("setup routes in a server to different directories")
#=>	@echo "Montrer qu'il y a un fichier par defaut" ("setup default file to search for if you ask for a directory")
	@echo "===== Different directories + default files "
	@echo "Check => 127.0.0.1:4141/index_files"
	@echo "Check => 127.0.0.1:4141/put_test"	
	@read -p "Press enter to continue..." continue
	
#=>	@echo "Montrer les list de method accepter" ("setup a list of method accepted for a certain route")	
	@echo "127.0.0.1:4141 only allow GET (view .conf file l.42)"
	@read -p "Press enter to continue..." continue
	-curl -X DELETE http://127.0.0.1:4141
	-curl -X PUT http://127.0.0.1:4141
	@read -p "Press enter to continue..." continue
	
	@echo "Methode GET"
	@read -p "Press enter to continue..." continue
	-curl -i -X GET http://127.0.2.1:4242
	@echo "Methode POST"
	@read -p "Press enter to continue..." continue
	-curl -i -X POST -H "Transfer-Encoding: chunked" --data-raw "$$(dd if=/dev/urandom bs=2000 count=1 status=none | base64 | tr -d '\n' | head -c 2000)" http://127.0.2.1:4242/directory/youpi.bla
	@echo "=> Check http://127.0.0.1:4141/put_test"
	@read -p "Press enter to continue..." continue
	@echo "Put some files to server:"
	-curl -i -X PUT --upload-file "John Denver Perhaps Love.mp3" http://127.0.0.1:4141/put_test/John_Denver_Perhaps_Love.mp3
	@echo "****************************************************************************"
	-curl -i -X PUT --upload-file "Hanoi.jpg" http://127.0.0.1:4141/put_test/Hanoi.jpg
	@echo "****************************************************************************"
	-curl -i -X PUT --upload-file "Break dance.mp4" http://127.0.0.1:4141/put_test/Break_dance.mp4
	@echo "=> Check http://127.0.0.1:4141/put_test"
	@read -p "Press enter to continue..." continue
	-curl -i -X DELETE http://127.0.0.1:4141/put_test/John_Denver_Perhaps_Love.mp3
	@echo "****************************************************************************"
	-curl -i -X DELETE http://127.0.0.1:4141/put_test/Hanoi.jpg
	@echo "****************************************************************************"
	-curl -i -X DELETE http://127.0.0.1:4141/put_test/Break_dance.mp4
	@echo "****************************************************************************"
	@echo "=> Recheck http://127.0.0.1:4141/put_test"
	@echo "Test with UNKNOWN method"
	@read -p "Press enter to continue..." continue
	-curl -i -X UNKNOWN http://127.0.2.1:4242
	@echo "=> Check request header and response header in => http://127.0.0.1:4141 (go in "reseaux")"
	@echo "=> Check wrong URL like http://127.0.0.1:4141/index_fil"
	@echo "=> Check listing http://127.0.0.1:4141/index_files"
	@echo "=> Check redirection http://127.0.0.1:5050/hoppy"
	@echo "=> Set up port multiple time = working, not written in subject (just If you’ve got a question about one behavior, you should compare your program behavior with NGINX’s.)"
	
#=>	@echo "Montrer deux website avec les memes ports et differente conf" ("launch multiple server at the same time with different configuration but with common port")
	@echo "=> Test Siege 2min - 25 threads"
	@read -p "Press enter to continue..." continue
	-siege -t2 -b 127.0.5.1:4141
	@echo "=> Bonus cookies + session"
	@echo "=> Check http://127.0.2.2:8000/test_cookie/"
	@read -p "Press enter to continue..." continue
	@echo "=> Test CGI"
	@read -p "Press enter to continue..." continue
	-curl -i -X GET 127.0.2.2:8000/hello.php
	-curl -i -X GET 127.0.2.2:8000/version.php
	-curl -i -X GET 127.0.2.2:8000/hello.js
	-curl -i -X GET 127.0.2.2:8000/hello.py


subjects:
	-make
	@echo "============================================================================"
	@echo "You must be able to serve a fully static website."
	@read -p "Press enter to continue..." continue
	@echo "=> Check http://127.0.0.1:4141"
	@read -p "Press enter to continue..." continue
	@echo "============================================================================"
	@echo "Clients must be able to upload files."
	@read -p "Press enter to continue..." continue
	@echo "=> Check http://127.0.0.1:4141/put_test"
	@read -p "Press enter to continue..." continue
	@echo "Put some files to server:"
	-curl -i -X PUT --upload-file "John Denver Perhaps Love.mp3" http://127.0.0.1:4141/put_test/John_Denver_Perhaps_Love.mp3
	@echo "****************************************************************************"
	-curl -i -X PUT --upload-file "Hanoi.jpg" http://127.0.0.1:4141/put_test/Hanoi.jpg
	@echo "****************************************************************************"
	-curl -i -X PUT --upload-file "Break dance.mp4" http://127.0.0.1:4141/put_test/Break_dance.mp4
	@echo "****************************************************************************"
	@echo "=> Recheck http://127.0.0.1:4141/put_test"
	@read -p "Press enter to continue..." continue
	@echo ""
	@echo "============================================================================"
	@echo "You need at least GET, POST, and DELETE methods."
	@read -p "Press enter to continue..." continue
	-curl -i -X GET http://127.0.2.1:4242
	@echo "****************************************************************************"
	@echo ""
	@read -p "Press enter to continue..." continue
	-curl -i -X POST -H "Transfer-Encoding: chunked" --data-raw "$$(dd if=/dev/urandom bs=2000 count=1 status=none | base64 | tr -d '\n' | head -c 2000)" http://127.0.2.1:4242/directory/youpi.bla
	@echo ""
	@read -p "Press enter to continue..." continue
	-curl -i -X DELETE http://127.0.0.1:4141/put_test/John_Denver_Perhaps_Love.mp3
	@echo "****************************************************************************"
	-curl -i -X DELETE http://127.0.0.1:4141/put_test/Hanoi.jpg
	@echo "****************************************************************************"
	-curl -i -X DELETE http://127.0.0.1:4141/put_test/Break_dance.mp4
	@echo "****************************************************************************"
	@echo "=> Recheck http://127.0.0.1:4141/put_test"
	@read -p "Press enter to continue..." continue
	@echo "Test with UNKNOWN method"
	@read -p "Press enter to continue..." continue
	-curl -i -X UNKNOWN http://127.0.2.1:4242
	@read -p "Press enter to continue..." continue
	@echo "============================================================================"
	@echo "Setup the server_names or not."
	@echo "Without server name"
	-curl -X GET http://127.0.3.1:8080
	@echo "****************************************************************************"
	@read -p "Press enter to continue..." continue
	-curl -H "Host: loic.com" http://127.0.3.1:8080
	@echo "****************************************************************************"
	@read -p "Press enter to continue..." continue
	-curl -H "Host: minh-ngu.42.fr" http://127.0.3.1:8080
	@echo "****************************************************************************"
	@read -p "Press enter to continue..." continue
	-curl -H "Host: nathan.fr" http://127.0.3.1:8080
	@echo "****************************************************************************"
	@read -p "Press enter to continue..." continue
	-curl -H "Host: minh-ngu.com" http://127.0.3.1:8080
	@read -p "Press enter to continue..." continue
	@echo "============================================================================"
	@echo "Limit client body size."
	-curl -i -X POST -H "Transfer-Encoding: chunked" --data-raw "$$(dd if=/dev/urandom bs=20 count=1 status=none | base64 | tr -d '\n' | head -c 20)" http://127.0.2.1:4242/post_body
	@read -p "Press enter to continue..." continue
	-curl -i -X POST -H "Transfer-Encoding: chunked" --data-raw "$$(dd if=/dev/urandom bs=120 count=1 status=none | base64 | tr -d '\n' | head -c 120)" http://127.0.2.1:4242/post_body
	@read -p "Press enter to continue..." continue
	@echo "Turn on or off directory listing."
	@echo "=> Check http://127.0.0.1:4141/index_files"
	@echo "=> Check http://127.0.0.1:4141/index_files0"
	@echo "=> Check http://127.0.0.1:4141/put_test"
	@echo "=> Check http://127.0.0.1:4141/put_test0"
	@read -p "Press enter to continue..." continue
	@echo "Execute CGI based on certain file extension (for example .php). Make it work with POST and GET methods."
	-curl -i -X GET 127.0.2.2:8000/hello.php
	-curl -i -X GET 127.0.2.2:8000/version.php
	@read -p "Press enter to continue..." continue
	@echo "Make the route able to accept uploaded files and configure where they should be saved."
	-curl -i -X PUT --upload-file "Hanoi.jpg" http://127.0.4.1:4343
	@echo "=> Check http://127.0.4.1:4343/index_files"
	-curl -i -X GET 127.0.2.2:8000/hello.js
	-curl -i -X GET 127.0.2.2:8000/hello.py
	@echo "============================================================================"
	@echo "=> Test .conf"
	@read -p "Press enter to continue..." continue
	@echo "=> The following tests should return error:"
	@read -p "Press enter to continue..." continue
	-valgrind --tool=memcheck --track-fds=yes --leak-check=full --show-leak-kinds=all --track-origins=yes ./webserv wrong_conf/.categorie_unknown.conf
	@echo "****************************************************************************"
	-valgrind --tool=memcheck --track-fds=yes --leak-check=full --show-leak-kinds=all --track-origins=yes ./webserv wrong_conf/host/send_error/.client_body_buffer_size_max.conf
	@echo "****************************************************************************"
	-valgrind --tool=memcheck --track-fds=yes --leak-check=full --show-leak-kinds=all --track-origins=yes ./webserv wrong_conf/host/send_error/.client_body_buffer_size_min.conf
	@echo "****************************************************************************"
	-valgrind --tool=memcheck --track-fds=yes --leak-check=full --show-leak-kinds=all --track-origins=yes ./webserv wrong_conf/host/send_error/.client_body_buffer_size_not.conf
	@echo "****************************************************************************"
	-valgrind --tool=memcheck --track-fds=yes --leak-check=full --show-leak-kinds=all --track-origins=yes ./webserv wrong_conf/host/send_error/.client_max_body_size_max.conf
	@echo "****************************************************************************"
	-valgrind --tool=memcheck --track-fds=yes --leak-check=full --show-leak-kinds=all --track-origins=yes ./webserv wrong_conf/host/send_error/.client_max_body_size_min.conf
	@echo "****************************************************************************"
	-valgrind --tool=memcheck --track-fds=yes --leak-check=full --show-leak-kinds=all --track-origins=yes ./webserv wrong_conf/host/send_error/.client_max_body_size_not.conf
	@echo "****************************************************************************"
	-valgrind --tool=memcheck --track-fds=yes --leak-check=full --show-leak-kinds=all --track-origins=yes ./webserv wrong_conf/host/send_error/.host_indentation.conf
	@echo "****************************************************************************"
	-valgrind --tool=memcheck --track-fds=yes --leak-check=full --show-leak-kinds=all --track-origins=yes ./webserv wrong_conf/host/send_error/.large_client_header_buffer_max.conf
	@echo "****************************************************************************"
	-valgrind --tool=memcheck --track-fds=yes --leak-check=full --show-leak-kinds=all --track-origins=yes ./webserv wrong_conf/host/send_error/.large_client_header_buffer_min.conf
	@echo "****************************************************************************"
	-valgrind --tool=memcheck --track-fds=yes --leak-check=full --show-leak-kinds=all --track-origins=yes ./webserv wrong_conf/host/send_error/.large_client_header_buffer_not.conf
	@echo "****************************************************************************"
	-valgrind --tool=memcheck --track-fds=yes --leak-check=full --show-leak-kinds=all --track-origins=yes ./webserv wrong_conf/host/send_error/.timeout_max.conf
	@echo "****************************************************************************"
	-valgrind --tool=memcheck --track-fds=yes --leak-check=full --show-leak-kinds=all --track-origins=yes ./webserv wrong_conf/host/send_error/.timeout_min.conf
	@echo "****************************************************************************"
	-valgrind --tool=memcheck --track-fds=yes --leak-check=full --show-leak-kinds=all --track-origins=yes ./webserv wrong_conf/host/send_error/.timeout_not.conf
	@echo "****************************************************************************"
	-valgrind --tool=memcheck --track-fds=yes --leak-check=full --show-leak-kinds=all --track-origins=yes ./webserv wrong_conf/host/send_error/.unknown_host.conf 
	@echo "****************************************************************************"
	-valgrind --tool=memcheck --track-fds=yes --leak-check=full --show-leak-kinds=all --track-origins=yes ./webserv wrong_conf/host/send_error/.workers_max.conf  
	@echo "****************************************************************************"
	-valgrind --tool=memcheck --track-fds=yes --leak-check=full --show-leak-kinds=all --track-origins=yes ./webserv wrong_conf/host/send_error/.workers_min.conf  
	@echo "****************************************************************************"
	-valgrind --tool=memcheck --track-fds=yes --leak-check=full --show-leak-kinds=all --track-origins=yes ./webserv wrong_conf/host/send_error/.workers_not.conf  
	@echo "****************************************************************************"
	-valgrind --tool=memcheck --track-fds=yes --leak-check=full --show-leak-kinds=all --track-origins=yes ./webserv wrong_conf/server/send_error/.listen_not.conf
	@echo "****************************************************************************"
	-valgrind --tool=memcheck --track-fds=yes --leak-check=full --show-leak-kinds=all --track-origins=yes ./webserv wrong_conf/server/send_error/.listen_wrong_ip.conf 
	@echo "****************************************************************************"
	-valgrind --tool=memcheck --track-fds=yes --leak-check=full --show-leak-kinds=all --track-origins=yes ./webserv wrong_conf/server/send_error/.listen_wrong_port.conf
	@echo "****************************************************************************"
	-valgrind --tool=memcheck --track-fds=yes --leak-check=full --show-leak-kinds=all --track-origins=yes ./webserv wrong_conf/server/send_error/.methods_wrong.conf
	@echo "****************************************************************************"
	-valgrind --tool=memcheck --track-fds=yes --leak-check=full --show-leak-kinds=all --track-origins=yes ./webserv wrong_conf/server/send_error/.root_not.conf 
	@echo "****************************************************************************"
	-valgrind --tool=memcheck --track-fds=yes --leak-check=full --show-leak-kinds=all --track-origins=yes ./webserv wrong_conf/server/send_error/.server_indentation.conf
	@echo "****************************************************************************"
	-valgrind --tool=memcheck --track-fds=yes --leak-check=full --show-leak-kinds=all --track-origins=yes ./webserv wrong_conf/server/send_error/.timeout_max.conf
	@echo "****************************************************************************"
	-valgrind --tool=memcheck --track-fds=yes --leak-check=full --show-leak-kinds=all --track-origins=yes ./webserv wrong_conf/server/send_error/.timeout_min.conf
	@echo "****************************************************************************"
	-valgrind --tool=memcheck --track-fds=yes --leak-check=full --show-leak-kinds=all --track-origins=yes ./webserv wrong_conf/server/send_error/.timeout_not.conf
	@echo "****************************************************************************"
	-valgrind --tool=memcheck --track-fds=yes --leak-check=full --show-leak-kinds=all --track-origins=yes ./webserv wrong_conf/server/send_error/.wrong_name_server_cat.conf
	@echo "****************************************************************************"
	-valgrind --tool=memcheck --track-fds=yes --leak-check=full --show-leak-kinds=all --track-origins=yes ./webserv wrong_conf/server/send_error/.autoindex_not.conf
	@echo "****************************************************************************"
	-valgrind --tool=memcheck --track-fds=yes --leak-check=full --show-leak-kinds=all --track-origins=yes ./webserv wrong_conf/server/send_error/.alias_not.conf
	@echo "****************************************************************************"
	-valgrind --tool=memcheck --track-fds=yes --leak-check=full --show-leak-kinds=all --track-origins=yes ./webserv wrong_conf/server/send_error/.redirection_one_arg.conf
	@echo "****************************************************************************"
	-valgrind --tool=memcheck --track-fds=yes --leak-check=full --show-leak-kinds=all --track-origins=yes ./webserv wrong_conf/server/send_error/.cgi_pass_not.conf
	@echo "****************************************************************************"
	-valgrind --tool=memcheck --track-fds=yes --leak-check=full --show-leak-kinds=all --track-origins=yes ./webserv wrong_conf/server/send_error/.location_not.conf
	@echo "=> The following tests should work"
	@read -p "Press enter to continue..." continue
	@echo "valgrind --tool=memcheck --track-fds=yes --leak-check=full --show-leak-kinds=all --track-origins=yes ./webserv wrong_conf/host/should_work/.no_host.conf"
	@echo "valgrind --tool=memcheck --track-fds=yes --leak-check=full --show-leak-kinds=all --track-origins=yes ./webserv wrong_conf/server/should_work/.listen_double_diff.conf"
	@echo "valgrind --tool=memcheck --track-fds=yes --leak-check=full --show-leak-kinds=all --track-origins=yes ./webserv wrong_conf/server/should_work/.methods_not.conf"
	@echo "valgrind --tool=memcheck --track-fds=yes --leak-check=full --show-leak-kinds=all --track-origins=yes ./webserv wrong_conf/server/should_work/.server_name_not.conf"
	@read -p "Press enter to continue..." continue
	@echo "============================================================================"
	@echo "=> Test Siege 2min - 25 threads"
	@read -p "Press enter to continue..." continue
	-siege -t2 -b 127.0.5.1:4141	
	@echo "=> Bonus cookies + session"
	@echo "=> Check http://127.0.2.2:8000/test_cookie/"
	@read -p "Press enter to continue..." continue
	@echo "!!!!!!!!!!!! To do: !!!!!!!!!!!!"
	@-cat to_check	
	@echo "!!!!!!!!!!!! REMOVE COMMENT MAKEFILE !!!!!!!!!!!!"


helgrind:
	clear && make re && make clean && valgrind --tool=helgrind --history-level=none ./webserv .conf

valgrind:
	clear && make re && make clean && valgrind --track-origins=yes --track-fds=yes --leak-check=full --show-leak-kinds=all ./webserv .conf

gits:
	git add Makefile
	git add *.cpp
	git add *.hpp
	git add .conf
	git add www
	git add tester
	git add error_pages
	git add "John Denver Perhaps Love.mp3"
	git add "Hanoi.jpg"
	git add "Break dance.mp4"
	git add ubuntu_tester
	git add ubuntu_cgi_tester
	git commit -m "defense in depth"
	git push

tester:
	clear && ./ubuntu_tester http://127.0.2.1:4242

wait:
	curl -i -X POST -H "Content-Type: application/json" -d '{"key1":"value1", "key2":"value2"}' 127.0.2.2:8000/wait.php

kill:
	pkill -9 -f "webserv .conf"

M:=
gitd:
	git add Makefile
	git add *.cpp
	git add *.hpp
	git add .conf
	git add www
	git add tester
	git add error_pages
	git add nginx.conf
	git add nginx.sh
	git add "John Denver Perhaps Love.mp3"
	git add "Hanoi.jpg"
	git add "Break dance.mp4"
	git commit -m "$(M)"
	git push
.PHONY: all clean fclean re test pause tester wait
