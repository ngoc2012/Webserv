/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Header.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/01/30 15:40:54 by lbastian         ###   ########.fr       */
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
    if (_status_code == 200)
    {
        str += "Allow: " + _allow + "\r\n";
        str += std::string("Content-Language: en") + "\r\n";
        if (_mimes->find(_extension) == _mimes->end())
            str += "Content-Type: text/html\r\n";
        else
            str += "Content-Type: " + (*_mimes)[_extension] + "\r\n";
    }
    else
    	str += "Content-Type: text/html\r\n";
    if (_status_code == 200 &&_response->get_request()->get_cgi())
        str += "Transfer-Encoding: chunked\r\n";
    else
	{
        str += "Content-Length: " + ft::itos(_response->get_content_length()) + "\r\n";
	}
	if (_session_id != "")
		str += "Set-Cookies: " + _session_id + "\r\n";
    str += "Date: " + get_current_time() + "\r\n";
	//std::cout << str << std::endl;
	str += "Access-Control-Allow-Origin: http://127.0.0.1:4141/\r\n";
	//str += "Access-Control-Allow-Origin: *\r\n";
	str += "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n";
	str += "Access-Control-Allow-Headers: Content-Type, Cookie, Authorization\r\n";
	str += "Access-Control-Allow-Credentials: true\r\n\r\n";
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
