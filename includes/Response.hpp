/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/01/02 16:42:59 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>

#ifndef RESPONSE_HPP
# define RESPONSE_HPP

class	Host;
class	Server;
class	Location;
class	Request;
class	Configuration;

class	Response
{
	private:
		int		        _socket;
		Host*		    _host;
		Server*		    _server;
		Request*	    _request;

		std::string	    _header;
		std::string	    _body;
		int		        _status_code;
		size_t		    _content_length;
		size_t		    _body_size;
		size_t		    _pos;

        int             _fd_out;
		std::string	    _full_file_name;
        bool            _write_queue;

        void	        write_header(void);
        void	        get_file_size(void);
        void	        mess_body(std::string, std::string);
		//void	        redir_body(std::string, std::string);

		void		    process_fd_out(void);
        int 	        write_body(void);

		int 		    end_connection(void);

		Response(const Response&);
		Response	&operator=(const Response& op);
	public:
		Response();
		virtual ~Response();

		int 		    write(void);

        int		        get_status_code(void) const;
		size_t		    get_content_length(void) const;
        int             get_fd_out(void) const;
        Host*		    get_host(void) const;
        Request*	    get_request(void) const;

		void		    set_socket(int);
		void		    set_host(Host*);
		void		    set_server(Server*);
		void		    set_request(Request*);
        void		    set_status_code(int);
        void            set_write_queue(bool);
        void            set_fd_out(int);
};

#endif
