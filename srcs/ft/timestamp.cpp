/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   timestamp.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/15 11:06:15 by ngoc              #+#    #+#             */
/*   Updated: 2024/02/07 10:31:07 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <ctime>

#include "webserv.hpp"

namespace ft {

void	timestamp(void)
{
    // std::time_t currentTime = std::time(0);
    // std::tm* currentTM = std::localtime(&currentTime);
    // std::cout << CYAN << "[" << currentTM->tm_hour << ":" << currentTM->tm_min << ":" << currentTM->tm_sec << "] " << RESET;
    std::time_t currentTime = std::time(0);
    std::tm currentTM;
    localtime_r(&currentTime, &currentTM); // Use localtime_r for thread safety
    std::cout << CYAN << "[" << currentTM.tm_hour << ":" << currentTM.tm_min << ":" << currentTM.tm_sec << "] " << RESET;
}
}