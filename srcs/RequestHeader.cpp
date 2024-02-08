/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHeader.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nbechon <nbechon@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/02/06 07:03:17 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sstream>

#include "Host.hpp"
#include "webserv.hpp"

#include "RequestHeader.hpp"

RequestHeader::RequestHeader() { }
RequestHeader::RequestHeader(const RequestHeader& src) { *this = src; }
RequestHeader&	RequestHeader::operator=( RequestHeader const & src )
{
	(void) src;
	return (*this);
}
RequestHeader::~RequestHeader() { }

bool	RequestHeader::parse_method_url(std::string& url, e_method& m)
{
    _pos = _str->find("\r\n");
    if (_pos == NPOS)
    {
        std::cerr << "Error: No newline." << std::endl;
        return (false);
    }
    std::istringstream      iss(_str->substr(0, _pos));
    std::string             tk;
    std::string             method;
    int i = 0;
    while (iss >> tk && i < 4)
    {
        if (i == 0)
            method = tk;
        if (i == 1)
            url = tk;
        i++;
    }
    if (i > 3)
    {
        std::cerr << "Error: First line header invalid." << std::endl;
        return (false);
    }
    if (method == "GET")
        m = GET;
    else if (method == "POST")
        m = POST;
    else if (method == "PUT")
        m = PUT;
    else if (method == "DELETE")
        m = DELETE;
	else if (method == "OPTIONS")
		m = OPTIONS;
    else if (method == "HEAD")
		m = HEAD;
    else
    {
        std::cerr << "Error: Method unknown : " << method << std::endl;
        return (false);
    }
    return (true);
}

std::string	    RequestHeader::parse_host_name()
{
    size_t  last_pos = _str->find("Host:", _pos);
    if (last_pos == NPOS)
    {
        std::cerr << "Error: Host name not found." << std::endl;
        return ("");
    }
    last_pos += 6;
    _pos = _str->find("\r\n", last_pos);
    if (_pos == NPOS)
    {
        std::cerr << "Error: No newline for host name." << std::endl;
        return ("");
    }
    return (_str->substr(last_pos, _pos - last_pos));
}

std::string	    RequestHeader::parse_content_type()
{
    size_t  last_pos = _str->find("Content-Type:", _pos);
    if (last_pos == NPOS)
    {
        //std::cerr << "Error: Content-Type not found." << std::endl;
        return ("");
    }
    last_pos += 14;
    size_t  pos = _str->find("\r\n", last_pos);
    if (pos == NPOS)
    {
        std::cerr << "Error: No newline for Content-Type." << std::endl;
        return ("");
    }
    std::string     type = _str->substr(last_pos, pos - last_pos);
    std::set<std::string>*	set_mimes = _host->get_set_mimes();
    if (set_mimes->find(type) != set_mimes->end())
        return (type);
    std::cerr << "Error: Content type " << type << " not found." << std::endl;
    return ("");
}

std::string	    RequestHeader::parse_connection()
{
    size_t  last_pos = _str->find("Connection:", _pos);
    if (last_pos == NPOS)
    {
        //std::cerr << "Error: Content-Type not found." << std::endl;
        return ("");
    }
    last_pos += 12;
    size_t  pos = _str->find("\r\n", last_pos);
    if (pos == NPOS)
    {
        std::cerr << "Error: No newline for Connection." << std::endl;
        return ("");
    }
    return (_str->substr(last_pos, pos - last_pos));
}

size_t	RequestHeader::parse_content_length()
{
    size_t  last_pos = _str->find("Content-Length:", _pos);
    if (last_pos == NPOS)
        return (0);
    last_pos += 16;
    size_t  pos = _str->find("\r\n", last_pos);
    if (pos == NPOS)
    {
        std::cerr << "Error: No newline for Content-Length." << std::endl;
        return (NPOS);
    }
    std::string     num = _str->substr(last_pos, pos - last_pos);
    pos = _str->find(";");
    if (pos != NPOS)
        num = num.substr(0, pos);
    if (!ft::is_digit(num))
    {
        std::cerr << "Error: Content-Length is not a number." << std::endl;
        return (NPOS);
    }
    return (std::atoi(num.c_str()));
}

bool    RequestHeader::parse_transfer_encoding()
{
    size_t  last_pos = _str->find("Transfer-Encoding:", _pos);
    if (last_pos == NPOS)
    {
        //std::cerr << "Error: Transfer-Encoding not found." << std::endl;
        return (false);
    }
    last_pos += 19;
    if (_str->substr(last_pos, 7) == "chunked")
        return (true);
    return (false);
}

//https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Cookie
//Cookie: name=value; name2=value2; name3=value3
//Cookie: PHPSESSID=298zf09hf012fh2; csrftoken=u32t4o3tb3gg43; _gat=1
std::map<std::string, std::string>    RequestHeader::parse_cookies()
{
    std::map<std::string, std::string>  cookies;
    (void) cookies;
    size_t  last_pos = _str->find("Cookie:", _pos);
    if (last_pos == NPOS)
        return (cookies);
    last_pos += 8;
    size_t  pos = _str->find("\r\n", last_pos);
    if (pos == NPOS)
    {
        std::cerr << "Error: No newline for Cookie." << std::endl;
        return (cookies);
    }
    std::string     str_cookies = _str->substr(last_pos, pos - last_pos);

    std::vector<std::string> cookie_pairs = ft::split_string(str_cookies, ";");

    for (std::vector<std::string>::iterator it = cookie_pairs.begin();
         it != cookie_pairs.end(); ++it)
    {
        // fonction trim_string pour éliminer les espaces
        std::string trimmed_cookie = ft::trim_string(*it);

        // séparer le nom et la valeur
        size_t equal_pos = trimmed_cookie.find('=');
        if (equal_pos != std::string::npos)
        {
            // Extraire le nom et la valeur
            std::string name = trimmed_cookie.substr(0, equal_pos);
            std::string value = trimmed_cookie.substr(equal_pos + 1);
            cookies[name] = value;
        }
    }
    return (cookies);
}

void	RequestHeader::set_host(Host* h) {_host = h;}
void    RequestHeader::set_str(std::string* s) {_str = s;}
void	RequestHeader::set_request(Request* r) {_request = r;}
