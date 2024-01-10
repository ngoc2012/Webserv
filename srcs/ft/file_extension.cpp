/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   file_extension.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/22 10:40:59 by ngoc              #+#    #+#             */
/*   Updated: 2023/12/27 14:54:33 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>

#include "webserv.hpp"

namespace ft {

std::string	file_extension(std::string file_path)
{
	size_t dot_position = file_path.find_last_of('.');
	if (dot_position != std::string::npos && dot_position < file_path.size() - 1)
		return (file_path.substr(dot_position + 1));
	return ("");
}

}
