/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nbechon <nbechon@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/14 22:38:19 by ngoc              #+#    #+#             */
/*   Updated: 2024/01/30 16:15:09 by nbechon          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstdlib>
#include <sys/stat.h>	// stat, S_ISDIR
#include <sys/select.h>	// select

#include <iostream>
#include <vector>
#include <map>
#include <fstream>

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# define NPOS std::string::npos

enum	e_method {GET, POST, PUT, DELETE, HEAD, OPTIONS, NONE};

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

}

#endif
