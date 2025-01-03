/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nbechon <nbechon@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/14 22:38:19 by ngoc              #+#    #+#             */
/*   Updated: 2024/03/05 14:00:23 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstdlib>
#include <sys/stat.h>	// stat, S_ISDIR
#include <sys/select.h>	// select

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <fstream>

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# define MEGABYTE 1048576
# define KILOBYTE 1024
# define RESPONSE_BUFFER 64
# define TIMEOUT 5
# ifndef RETRY
#   define RETRY 5
# endif
# ifndef DELAY
#   define DELAY 0 // ms * n_workers
# endif

# ifndef RUPTURE
#   define RUPTURE 0 // Close connection after send or recv failure
# endif

# define NPOS std::string::npos

// ANSI color codes
// https://gist.github.com/RabaDabaDoba/145049536f815903c79944599c6f952a
# define RESET   "\033[0m"
# define BLACK   "\033[30m"
# define RED     "\033[31m"
# define GREEN   "\033[32m"
# define YELLOW  "\033[33m"
# define BLUE    "\033[34m"
# define MAGENTA "\033[35m"
# define CYAN    "\033[36m"
# define WHITE   "\033[37m"
# define BOLD    "\033[1m"
# define UNDERLINE "\033[4m"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

enum	e_method {GET, POST, PUT, DELETE, HEAD, OPTIONS, NONE};
enum    e_message {NORMAL, WARNING, ERROR, SUCCESS};

namespace ft {

std::vector<std::string>	split_string(const std::string& input, std::string delimiters);
bool				        is_digit(std::string& str);
std::string			        itos(int n);
bool                        match_wildcard(const char* str, const char* pattern);
std::string	                file_extension(std::string);
size_t                      atoi_base(const char *str, const char *base);
std::string                 itoa_base(int nbr, const char *base);
std::string                 str_replace(std::string src, std::string find, std::string rep);
std::string                 to_upper(std::string s);
std::string                 trim_string(const std::string& str);
void	                    timestamp(void);
void                        print_loading_bar(int progress, int total, int barWidth, pthread_mutex_t* _cout_mutex);
void                        print_size(size_t size, pthread_mutex_t* _cout_mutex);
char*						strdup(const char* str);
std::string                 convertisseur(size_t size);
}

#endif
