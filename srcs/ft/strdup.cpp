/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   strdup.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/05 13:58:50 by ngoc              #+#    #+#             */
/*   Updated: 2024/03/05 13:59:55 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstring>
#include <cstdlib>

namespace ft {

char*	strdup(const char* str)
{
    char*   newStr = (char*)std::malloc(std::strlen(str) + 1);
    if (newStr)
        std::strcpy(newStr, str);
    return newStr;
}
}