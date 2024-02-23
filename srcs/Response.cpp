/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nbechon <nbechon@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/02/23 05:48:07 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <time.h>

#include <sys/types.h>
#include <sys/wait.h>

#include "Host.hpp"
#include "Server.hpp"
#include "Location.hpp"
#include "Request.hpp"
#include "Cgi.hpp"
#include "webserv.hpp"

#include "Response.hpp"

Response::Response() { init(); }
Response::Response(const Response& src) { *this = src; }
Response&	Response::operator=( Response const & src )
{
	(void) src;
	return (*this);
}
Response::~Response()
{
    if (_fd_out != -1)
        close(_fd_out);
}

void    Response::init(void)
{
    _header = "";
    _body = "";
    _status_code = 200;
    _content_type = "";
    _content_length = 0;
    _body_size = 0;
    _pos = 0;
    _full_file_name = "";
    _fd_out = -1;
    _end_header = false;
}

int     Response::write()
{
    if (!_end_header)
        write_header();
    else
        write_body();
    return (0);
}

void     Response::write_header()
{
    std::cout << "write_header" << std::endl;
    int     ret = send(_socket, _header.c_str(), _header.length(), 0);
    std::cout << "Response Header:\n'" << _header << "`" << _header.size() << "|" << ret << std::endl;
    if (ret <= 0)
        end_response();
    _host->set_sk_timeout(_socket);
    _header.erase(0, ret);
    if (_header == "")
    {
        _end_header = true;
        if (!_content_length || _request->get_method() == HEAD)
            end_response();
    }
}

void     Response::header_generate()
{
    _full_file_name = _request->get_full_file_name();
    //std::cout << "write_header " << _full_file_name << std::endl;
    Header	header(this);
    if (_status_code == 405)
        header.set_allow(_request->get_location()->get_methods_str());
    body_generate();
    if (_request->get_method() == HEAD && _status_code == 200)
        _content_length = 0;
    else if (_body != "")
        _content_length = _body.size();
    set_session_id(header);
    _header = header.generate();
}

void	 Response::body_generate(void)
{
    if (_status_code != 200)
    {
        if (_status_code == 301 || _status_code == 302)
        {
            mess_body(ft::itos(_status_code) + " Redirection",
                   "This page has moved. If you are not redirected, <a href=\""
                   + _request->get_location()->get_link() + "\">click here</a>.");
        } else {
            std::string mess = (*_host->get_status_message())[_status_code];
            mess_body(ft::itos(_status_code) + " " + mess, mess);
        }
    }
    else if (_request->get_method() == DELETE)
        mess_body("Delete", "File deleted");
}

void     Response::set_session_id(Header& header)
{
    std::string sid = _request->get_session_id();
    if (sid != "")
    {
        Sessions*       sessions = _server->get_sessions();
        if (!sessions->verify(sid))
        {
            std::string token = sessions->create_token();
            sessions->add(token, std::time(0) + 30);
            header.set_session_id(token);
        }
        else
        {
            sessions->add(sid, std::time(0) + 30);
            header.set_session_id(sid);
        }
    }
}

void     Response::mess_body(std::string title, std::string body)
{
    _body += "<!DOCTYPE html>\n";
    _body += "<html>\n";
    _body += "  <head>\n";
    _body += "      <title>" + title + "</title>\n";
    _body += "  </head>\n";
    _body += "  <body>\n";
    _body += "      <p>" + body + "</p>\n";
    _body += "  </body>\n";
    _body += "</html>\n";
    _content_length = _body.size();
}

int     Response::write_body()
{
    int     ret;
    //std::cout << "write_body " << std::endl;
    if (_body != "")
    {
        //std::cout << "write_body " << _pos << " " << len << " " << _body << std::endl;
        ret = send(_socket, _body.c_str(), _body.size(), 0);
        if (ret <= 0)
        {
            if (ret == -1)
                std::cerr << RED << "Error: Send body." << RESET << std::endl;
            return (end_response());
        }
        _host->set_sk_timeout(_socket);
        _body_size += ret;
        _body.erase(0, ret);
        if (_body == "")
            return (end_response());
        return (0);
    }
    if (_fd_out == -1)
        return (end_response());
    //std::cerr << "Response _fd_out:" << _fd_out << std::endl;
    char	buffer[RESPONSE_BUFFER * 1028 + 20];
    ret = read(_fd_out, buffer, RESPONSE_BUFFER * 1028);
    //std::cout << _request->get_cgi() << std::endl;
    if (ret <= 0)
    {
        if (ret == -1)
            std::cerr << RED << "Error: Read fd_out." << RESET << std::endl;
        return (end_response());
    }
    //buffer[ret] = 0;
    //std::cout << ret << ":" << std::string(buffer).substr(0, 100) << std::endl;
    int     ret1 = send(_socket, buffer, ret, 0);
    //std::cout << "_body_size: " << _body_size << ", ret = " << ret1 << std::endl;
    // if (_full_file_name == "tester/YoupiBanane/youpi.bla" && _content_length == 100000)
    //     std::cout << "_buffer: `" << buffer << "`" << std::endl;
    // else
    //     std::cout << "_buffer: `" << std::string(buffer).substr(0, 100) << "`" << std::endl;
    if (ret1 <= 0)
    {
        if (ret1 == -1)
            std::cerr << "Send error ret = " << ret1 << std::endl;
        return (end_response());
    }
    _host->set_sk_timeout(_socket);
    _body_size += ret1;
    if (_body_size >= _content_length)
        return (end_response());
    return (0);
}

int     Response::end_response(void)
{
    //std::cout << "end_response " << _socket << " " << _full_file_name << std::endl;
    if (_fd_out > 0)
        close(_fd_out);
    //_write_queue = false;
    ft::timestamp();
    if (_status_code == 200)
        std::cout << GREEN;
    else
        std::cout << RED;
    std::cout << _status_code << " ";
    std::cout << _request->get_url() << " ";
    std::cout << _request->get_location()->get_method_str(_request->get_method()) << RESET << std::endl;
    std::cout << "Response _body_size: " << _body_size << std::endl;
    //usleep(5);
    //_host->close_client_sk(_socket);
    init();
    _request->init();
    _host->renew_request_sk(_socket);
    return (0);
}

int		        Response::get_status_code(void) const {return (_status_code);}
int             Response::get_fd_out(void) const {return (_fd_out);}
Host*		    Response::get_host(void) const {return (_host);}
Request*	    Response::get_request(void) const {return (_request);}
bool            Response::get_end_header(void) const {return (_end_header);}
size_t		    Response::get_content_length(void) const {return (_content_length);}
//std::string*	Response::get_body(void) {return (&_body);};
std::string	    Response::get_content_type(void) const {return (_content_type);}

void		Response::set_socket(int s) {_socket = s;}
void		Response::set_host(Host* h) {_host = h;}
void		Response::set_server(Server* s) {_server = s;}
void		Response::set_request(Request* r) {_request = r;}
void		Response::set_status_code(int e) {_status_code = e;}
void        Response::set_fd_out(int f) {_fd_out = f;}
void		Response::set_content_length(size_t l) {_content_length = l;}
void	    Response::set_body(std::string s) {_body = s;}
void        Response::set_content_type(std::string t) {_content_type = t;}
