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

void print_size(size_t size, pthread_mutex_t* _cout_mutex)
{
    if (_cout_mutex)
        pthread_mutex_lock(_cout_mutex);

    std::cout << size << "b\r";
    std::cout.flush();

    if (_cout_mutex)
        pthread_mutex_unlock(_cout_mutex);
}

}
