/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   convertisseur.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nbechon <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/23 08:42:44 by nbechon           #+#    #+#             */
/*   Updated: 2023/12/23 15:22:23 by nbechon          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <string>
#include <sstream>

namespace ft {

std::string     convertisseur(size_t size) {
    std::ostringstream result;
    if (size < 1024) {
        result << size << "b";
    } else if (size < 1024 * 1024) {
        result << size / 1024 << "Kb";
    } else {
        result << size / (1024 * 1024) << "Mb";
    }
    return result.str();
}

}