/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHeader.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/02/06 07:00:02 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUESTHEADER_HPP
# define REQUESTHEADER_HPP
 
class	Host;
class	Request;

class	RequestHeader
{
	private:
		Request*		                            _request;
		Host*		                                _host;
		std::string*                                _str;
		size_t		                                _pos;
        std::map<std::string, std::string>		    _tags;

		RequestHeader(const RequestHeader&);
		RequestHeader		&operator=(const RequestHeader& op);
	public:
		RequestHeader();
		virtual ~RequestHeader();

        bool            parse_method_url(std::string&, e_method&);
        std::string     parse_host_name();
        std::string     parse_content_type();
        size_t          parse_content_length();
        bool            parse_transfer_encoding();
        std::string     parse_connection();

		std::map<std::string, std::string>		    parse_cookies();

        void	set_host(Host*);
        void    set_str(std::string*);
        void	set_request(Request*);
};

#endif
