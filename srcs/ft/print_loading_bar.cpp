/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print_loading_bar.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/24 14:53:22 by ngoc              #+#    #+#             */
/*   Updated: 2024/02/26 17:09:20 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <sstream>

#include "webserv.hpp"

namespace ft {

void print_loading_bar(int progress, int total, int barWidth = 50)
{
    float percentage = static_cast<float>(progress) / total;
    int numChars = static_cast<int>(percentage * barWidth);

    std::cout << "[";
    for (int i = 0; i < barWidth; ++i) {
        if (i < numChars) {
            std::cout << "=";
        } else {
            std::cout << " ";
        }
    }
    std::cout << "] " << static_cast<int>(percentage * 100.0) << "%\r";
    std::cout.flush();
}

}
