/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Header.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/01/26 12:24:19 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ctime>	// time

#include <iostream>
#include <iomanip>	 // strftime, gmtime

#ifndef HEADER_HPP
# define HEADER_HPP
 
class	Server;
class	Location;
class	Request;
class	Configuration;
class	Response;

class	Header
{
	private:
		int					                _status_code;
		Host*		                        _host;
		Response*				            _response;
		std::map<int, std::string>*		    _status_message;
        
		std::string				            _allow;
		std::string				            _extension;
		std::string				            _session_id;
		

		std::string			                get_current_time(void);
		std::string			                file_last_modified_time(std::string);
		//void				                init(void);

		Header();
		Header(const Header&);
		Header		&operator=(const Header& op);

	public:
		Header(Response*);
		virtual ~Header();

		std::string			generate(void);

        void				set_status_code(int);
		void				set_allow(std::string);
        void			    set_session_id(std::string);
};

#endif
