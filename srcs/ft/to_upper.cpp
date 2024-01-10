/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   to_upper.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/27 15:28:09 by ngoc              #+#    #+#             */
/*   Updated: 2023/12/27 15:51:31 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>

namespace ft {

std::string to_upper(std::string s)
{
    for (std::string::iterator it = s.begin();
            it != s.end(); ++it)
        *it = std::toupper(*it);
    return (s);
}

}
