/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHeader.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/01/08 08:50:32 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUESTHEADER_HPP
# define REQUESTHEADER_HPP
 
class	Host;

class	RequestHeader
{
	private:
		Host*		    _host;
		std::string*    _str;
		size_t		    _pos;

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

        void	set_host(Host*);
        void    set_str(std::string*);
};

#endif
