/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Sessions.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lbastian <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/11 17:08:35 by lbastian          #+#    #+#             */
/*   Updated: 2024/01/11 18:47:59 by lbastian         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SESSIONS_HPP
# define SESSIONS_HPP

#include <time.h>

#include <map>
#include <iostream>

class Sessions
{
	private:
		Sessions(const Sessions& sessions);
		Sessions& operator=(const Sessions& sessions);
		std::map<std::string, std::map<int, time_t> > _sessions_data;

	public:
		Sessions(void);
		~Sessions(void);
		int		create_token(void);
		void	add(std::string username, int token, time_t date);
		bool	verify(std::string username, int token);
};


#endif
