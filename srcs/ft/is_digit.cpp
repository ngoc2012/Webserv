/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   is_digit.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/15 11:06:15 by ngoc              #+#    #+#             */
/*   Updated: 2024/01/05 21:31:13 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cctype>
#include <string>

#include "webserv.hpp"

namespace ft {

bool	is_digit(std::string& str)
{
	for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
		if (!isdigit(*it))
			return false;
	return true;
}

}
