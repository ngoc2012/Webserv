/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   split_string.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/14 22:14:49 by ngoc              #+#    #+#             */
/*   Updated: 2024/01/05 21:46:45 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <vector>
#include "webserv.hpp"

namespace ft {

std::vector<std::string>	split_string(const std::string& input, std::string delimiters)
{
	std::vector<std::string>	tokens;
    std::size_t                 i;
    std::size_t                 start;

    i = 0;
    start = 0;
    while (i < input.size())
    {
		while (i < input.size() && delimiters.find(input[i]) != std::string::npos)
            i++;
        start = i;
		while (i < input.size() && delimiters.find(input[i]) == std::string::npos)
            i++;
        if (i > start)
            tokens.push_back(input.substr(start, i - start));
    }
	return (tokens);
}

}
