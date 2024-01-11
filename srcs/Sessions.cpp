/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Sessions.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbastian <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/11 17:07:16 by lbastian          #+#    #+#             */
/*   Updated: 2024/01/11 18:48:10 by lbastian         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Sessions.hpp"

Sessions::Sessions(void)
{
}

Sessions::~Sessions(void)
{
}

int Sessions::create_token(void)
{
	return (_sessions_data.size() + 1);
}

void Sessions::add(std::string username, int token, time_t date)
{
	_sessions_data[username].insert(std::make_pair(token, date));
}

bool Sessions::verify(std::string username, int token)
{
	std::map<std::string, std::map<int, time_t> >::iterator it;
	std::map<int, time_t>::iterator it_inside;
	time_t actual;

	it = _sessions_data.find(username);
	if (it == _sessions_data.end())
		return (false);
	it_inside = it->second.find(token);
	if (it_inside == it->second.end())
		return (false);
	actual = time(0);
	if (it_inside->second < actual)
		return (false);
	return (true);
}
