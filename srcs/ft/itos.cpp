/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   itos.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/24 14:53:22 by ngoc              #+#    #+#             */
/*   Updated: 2023/12/23 08:28:04 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <sstream>

#include "webserv.hpp"

namespace ft {

std::string	itos(int n)
{
	std::ostringstream oss;
	oss << n;
	return (oss.str());
}

}
