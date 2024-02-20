/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Header.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nbechon <nbechon@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/01/30 16:48:59 by nbechon          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Host.hpp"
#include "Server.hpp"
#include "Location.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "webserv.hpp"

#include "Header.hpp"

Header::Header() { }
Header::Header(const Header& src) { *this = src; }
Header&	Header::operator=( Header const & src )
{
	(void) src;
	return (*this);
}
Header::Header(Response* r, std::string ext) :
	_response(r),
	_extension(ext)
{
    _host = _response->get_host();
    _mimes = _host->get_mimes();
    _status_message = _host->get_status_message();
	_session_id = "";
	init();
}
Header::~Header() { }

std::string	Header::generate(void)
{
	std::string	str;

    _status_code = _response->get_status_code();
	str = "HTTP/1.1 ";
	str += ft::itos(_status_code) + " ";
	if (_status_message->find(_status_code) == _status_message->end())
		str += "Unknown error code";
	else
		str += (*_status_message)[_status_code];
	str += "\r\n";
	if (_status_code == 405)
		str += "Allow: " + _allow + "\r\n";
    if (_status_code == 200)
    {
        
        str += std::string("Content-Language: en") + "\r\n";
		Request*	_request = _response->get_request();
		if (_request->get_cgi() && _extension == "js")
			str += "Content-Type: application/json\r\n";
        else if (_mimes->find(_extension) == _mimes->end())
            str += "Content-Type: plain/text\r\n";
        else
            str += "Content-Type: " + (*_mimes)[_extension] + "\r\n";
    }
    else
    	str += "Content-Type: text/html\r\n";
    if (_response->get_content_length() > 0)
        str += "Content-Length: " + ft::itos(_response->get_content_length()) + "\r\n";
    std::cout << "===============" << _session_id << std::endl;
    if (_session_id != "")
        str += "Set-Cookies: session_id=" + _session_id + "\r\n";
    str += "Date: " + get_current_time() + "\r\n";
    str += "server: webserv\r\n\r\n";
    return (str);
}

void	Header::init(void)
{
}

std::string	Header::get_current_time(void)
{
	std::time_t currentTime = std::time(0);
	std::tm* timeInfo = std::gmtime(&currentTime);
	char buffer[80];
	std::strftime(buffer, 80, "%a, %d %b %Y %H:%M:%S GMT", timeInfo);
	return (std::string(buffer));
}

std::string	Header::file_last_modified_time(std::string file_name)
{
	struct stat	file_info;
	if (stat(file_name.c_str(), &file_info) != 0) {
		std::cerr << "Error getting file information." << std::endl;
		return ("");
	}
	std::tm*	time_info = std::gmtime(&file_info.st_mtime);
	char		buffer[80];
	std::strftime(buffer, 80, "%a, %d %b %Y %H:%M:%S GMT", time_info);
	return (std::string(buffer));
}

void		Header::set_status_code(int s) {_status_code = s;}
void		Header::set_allow(std::string a) {_allow = a;}
void	    Header::set_session_id(std::string s) {_session_id = s;}
