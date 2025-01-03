/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Sessions.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbastian <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/11 17:07:16 by lbastian          #+#    #+#             */
/*   Updated: 2024/01/30 16:28:07 by lbastian         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Sessions.hpp"

Sessions::Sessions(void)
{
	pthread_mutex_init(&_sessions_mutex, NULL);
}

Sessions::~Sessions(void)
{
	pthread_mutex_destroy(&_sessions_mutex);
}

std::string Sessions::create_token(void)
{
	std::string str;
	std::map<std::string, time_t>::iterator it;
	time_t currentTime;

	while (1)
	{
		currentTime = time(NULL);
    	std::stringstream timeStream;
    	timeStream << currentTime;
		str = timeStream.str() + ft::itos(_sessions_data.size() + 1);

		it = _sessions_data.find(str);
		if (it == _sessions_data.end())
			break;
	}
	return (str);

}

void Sessions::add(std::string token, time_t date)
{
	_sessions_data[token] = date;
}

bool Sessions::verify(std::string token)
{
	std::map<std::string, time_t>::iterator it;
	std::map<int, time_t>::iterator it_inside;
	time_t actual;

	it = _sessions_data.find(token);
	if (it == _sessions_data.end())
		return (false);
	actual = time(0);
	if (_sessions_data[token] < actual)
	{
		_sessions_data.erase(token);
		return (false);
	}
	return (true);
}

int Sessions::check_date(int day, int month, int year)
{
	int days_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	
	if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)
		if (month == 2 && day == 29)
			return (0);
	if (day == 0 || day > days_month[month - 1])
		return (1);
	return (0);

}

time_t Sessions::get_time_t(int sec, int min, int hour, int day, int month, int year)
{
	time_t result;
	tm tm = {};

	if (sec > 59 || sec < 0 || min > 59 || min < 0 || hour < 0 || hour > 23 || day < 1 || day > 31 || month < 1 || month > 12 || year < 1900 || check_date(day, month, year))
		return (-1);
	tm.tm_sec = sec;
	tm.tm_min = min;
	tm.tm_hour = hour;
	tm.tm_mday = day;
	tm.tm_mon = month - 1;
	tm.tm_year = year - 1900;
	result = mktime(&tm);
	return (result);
}

pthread_mutex_t*			Sessions::get_sessions_mutex(void) {return (&_sessions_mutex);}