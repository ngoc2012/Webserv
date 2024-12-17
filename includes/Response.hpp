/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/02/29 15:15:12 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>

#include "Header.hpp"

#ifndef RESPONSE_HPP
# define RESPONSE_HPP

class	Host;
class	Worker;
class	Server;
class	Location;
class	Request;
class	Configuration;

class	Response
{
	private:
		int		        _socket;
		Host*		    _host;
		Worker*		    _worker;
		Server*		    _server;
		Request*	    _request;

		std::string	    _header;
		std::string	    _body;
		int		        _status_code;
		std::string	    _content_type;
		size_t		    _content_length;
		size_t		    _body_size;
        bool            _end_header;
		bool            _end;

        int             _fd_out;
    	char		_buffer[RESPONSE_BUFFER * 1028 + 20];

        int		        write_header(void);
        void	        get_file_size(void);
		void			body_generate(void);
		void			set_session_id(Header&);
        void	        mess_body(std::string, std::string);

        int 	        write_body(void);
        int 	        write_str_body(void);
        int 	        read_fd_out(void);

		void            init(void);
		int 		    end_response(int);

		Response(const Response&);
		Response	&operator=(const Response& op);
	public:
		Response();
		virtual ~Response();

		int 		    write(void);
		void			header_generate(void);

        int		        get_status_code(void) const;
        int             get_fd_out(void) const;
        Host*		    get_host(void) const;
        Request*	    get_request(void) const;
        bool            get_end_header(void) const;
		size_t		    get_content_length(void) const;
        std::string	    get_content_type(void) const;
		std::string*	get_header(void);
        bool            get_end(void) const;

		void		    set_socket(int);
		void		    set_host(Host*);
        void		    set_worker(Worker*);
		void		    set_server(Server*);
		void		    set_request(Request*);
        void		    set_status_code(int);
        void            set_fd_out(int);
		void			set_content_length(size_t);
		void			set_body(std::string);
        void            set_content_type(std::string);
};

#endif
