/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/02/21 10:47:55 by minh-ngu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

#include "RequestHeader.hpp"
#include "Response.hpp"

#ifndef REQUEST_HPP
# define REQUEST_HPP

class	Host;
class	Address;
class	Server;
class	RequestHeader;
class	Response;
class	Cgi;
enum    e_method;

class	Request
{
	private:
		int		        _socket;
		Host*		    _host;
		Address*		_address;
		Server*		    _server;
		RequestHeader	_header;
		Response	    _response;
		Location*	    _location;	
        Cgi*            _cgi;

		std::string	    _str_header;
		std::string	    _url;
		std::string	    _host_name;
		e_method	    _method;
		std::string	    _content_type;
		size_t		    _content_length;
		bool		    _close;
		bool		    _end_header;
		bool		    _end_chunked_body;
		bool		    _end;
		std::string	    _read_data;
		size_t      	_chunk_size;
		bool		    _chunked;
		size_t			_header_size;
		size_t		    _body_size;
        size_t 		    _body_left;
		std::map<std::string, std::string>  _cookies;
		std::string	    _session_id;

		int		        _fd_in;
		std::string	    _full_file_name;
		size_t		    _body_buffer;
		size_t		    _body_max;
        char*   	    _buffer;
		size_t			_buffer_size;
		size_t			_header_buffer;
        std::string     _tmp_file;

		int		        _status_code;

		int 		    read_header(void);
		int			    receive_header(void);
		bool		    parse_header(void);
        bool	        check_location(void);
		bool	        check_session(void);

		void		    process_fd_in(void);
        int 	        read_body();
        bool 	        write_chunked();

		int             end_request(void);
        void            envs(void);
        void            exec(void);
		void            clean(void);

		Request();
		Request(const Request&);
		Request &operator=(const Request& op);
	public:
		Request(int, Host*, Address*);
		virtual ~Request();

		int             read(void);
		void            init(void);

        Host*		    get_host(void) const;
        Server*		    get_server(void) const;
		e_method	    get_method(void) const;
		std::string	    get_url(void) const;
		Response*	    get_response(void);
        Cgi*            get_cgi(void) const;
		int		        get_status_code(void) const;
        std::string	    get_content_type(void) const;
        size_t		    get_content_length(void) const;
        size_t		    get_body_size(void) const;
        std::string	    get_str_header(void) const;
        std::string	    get_full_file_name(void) const;
        Location*	    get_location(void) const;	
        int		        get_fd_in(void) const;
        std::string	    get_session_id(void) const;
        bool		    get_end(void) const;
        bool		    get_end_header(void) const;

        void		    set_fd_in(int);
        //void		    set_end(bool);
};

#endif
