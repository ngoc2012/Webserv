/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nbechon <nbechon@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/02/21 20:48:27 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>

#ifndef CGI_HPP
# define CGI_HPP

class	Request;

class	Cgi
{
	private:
        std::string     _pass;
        std::string     _file;
		Request*        _request;
        Response*       _response;
        char**          _envs;
        int             _pid;
        std::string     _tmp_file;
        int             _fd_out;
        int             _status_code;
        std::string     _status_message;
        std::string     _content_type;
        size_t          _content_length;

        bool            get_envs();

		Cgi();
		Cgi(const Cgi&);
		Cgi	&operator=(const Cgi& op);
	public:
		Cgi(Request*);
		virtual ~Cgi();

        int             execute();
        int             parse_header();

        int             get_pid(void) const;
        int             get_status_code(void) const;
        std::string     get_status_message(void) const;
        std::string     get_content_type(void) const;
        size_t          get_content_length(void) const;

        void	        set_request(Request*);
        void            set_pass(std::string);
		void        	set_file(std::string f);
};

#endif
