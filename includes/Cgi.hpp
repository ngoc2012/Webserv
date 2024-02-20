/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nbechon <nbechon@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/01/26 10:56:20 by ngoc             ###   ########.fr       */
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
        char**          _envs;
        int             _pid;
        std::string     _tmp_file;
        int             _fd_out;

        bool            get_envs();

		Cgi();
		Cgi(const Cgi&);
		Cgi	&operator=(const Cgi& op);
	public:
		Cgi(Request*);
		virtual ~Cgi();

        int             execute();

        int             get_pid(void) const;

        void	        set_request(Request*);
        void            set_pass(std::string);
		void        	set_file(std::string f);
};

#endif
