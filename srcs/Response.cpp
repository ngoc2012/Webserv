/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nbechon <nbechon@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/02/29 15:15:42 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <time.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <cerrno> // For errno
#include <cstring> // For strerror

#include <iostream>
#include <fstream>
#include <sstream>

#include "Host.hpp"
#include "Worker.hpp"
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
    {
	_host->clear_read_fd(_fd_out);
        pthread_mutex_lock(_host->get_fd_mutex());
        close(_fd_out);
        _fd_out = -1;
        pthread_mutex_unlock(_host->get_fd_mutex());
    }
        
}

void    Response::init(void)
{
    _header = "";
    _body = "";
    _status_code = 200;
    _content_type = "";
    _content_length = 0;
    _body_size = 0;
    _fd_out = -1;
    _end_header = false;
    _end = false;
}

int     Response::write()
{
    if (!_end_header)
        return (write_header());
    else
        return (write_body());
    return (0);
}

int     Response::write_header()
{
    int     ret = send(_socket, _header.c_str(), _header.length(), 0);
    if (ret <= 0)
        return (ret);
    _worker->set_sk_timeout(_socket);
    _header.erase(0, ret);
    if (_header == "")
    {
        _end_header = true;
        if (!_content_length || _request->get_method() == HEAD)
            return (end_response(ret));
    }
    if (_request->get_method() == HEAD)
        return (end_response(ret));
    return (ret);
}

void     Response::header_generate()
{
    Header	header(this);
    if (_status_code == 405)
        header.set_allow(_request->get_location()->get_methods_str());
    if (_status_code != 200 || (_status_code == 200 && _request->get_method() == DELETE))
        _content_type = "text/html";
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
        // Redirection
        if (_status_code > 300 && _status_code < 400)
            return ;
        else
        {
            std::string file_name = "error_pages/" + ft::itos(_status_code) + ".html";
            std::ifstream file(file_name.c_str());
            if (file.is_open())
            {
                std::stringstream buffer;
                buffer << file.rdbuf();
                _body += buffer.str();
                file.close();
            }
            else
            {
                std::string mess = (*_host->get_status_message())[_status_code];
                mess_body(ft::itos(_status_code) + " " + mess, mess);
            }
        }
    }
    else if (_request->get_method() == DELETE)
        mess_body("Delete", "File deleted");
}

void     Response::set_session_id(Header& header)
{
    pthread_mutex_t*    mutex = _server->get_sessions()->get_sessions_mutex();
    pthread_mutex_lock(mutex);
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
    pthread_mutex_unlock(mutex);
}

void     Response::mess_body(std::string title, std::string body)
{
    if (_body != "")
        return ;
    _body += "<!DOCTYPE html>\n";
    _body += "<html>\n";
    _body += "  <head>\n";
    _body += "      <title>" + title + "</title>\n";
    _body += "  </head>\n";
    _body += "  <body>\n";
    _body += "      <p>" + body + "</p>\n";
    _body += "  </body>\n";
    _body += "</html>\n";
}

int     Response::write_str_body()
{
	int	ret;

        ret = send(_socket, _body.c_str(), _body.size(), 0);
        if (ret <= 0)
        {
            if (!ret && _body.size())
        	_host->print(ERROR, "Error: Send response body interrupted.");
            if (ret == -1)
                std::cerr << RED << "Error: Send body error." << RESET << std::endl;
            return (end_response(ret));
        }
        _worker->set_sk_timeout(_socket);
        _body_size += ret;
        _body.erase(0, ret);
        if (_body == "")
            return (end_response(ret));
        return (ret);
}

int     Response::write_body()
{
    int     ret;
    if (_body != "")
	    return write_str_body();
    if (_fd_out == -1)
    {
	_host->print(ERROR, "Error: No file content to send, " + ft::itos(_body_size) + "b sent.");
        return (end_response(-1));
    }
    char	buffer[RESPONSE_BUFFER * 1028 + 20];
    ret = read(_fd_out, buffer, RESPONSE_BUFFER * 1028);
    if (ret <= 0)
    {
        pthread_mutex_lock(_host->get_cout_mutex());
        if (ret == -1)
        {
            std::cerr << RED << "Error: Read fd out: " << _fd_out << "|" << strerror(errno) << RESET << std::endl;
            if (errno == ENOENT) {
                std::cerr << "The file does not exist." << std::endl;
            } else if (errno == EACCES) {
                std::cerr << "Permission denied." << std::endl;
            }
        }
        if (!ret)
            std::cerr << RED << "Error: Nothing more to send." << RESET << std::endl;
        pthread_mutex_unlock(_host->get_cout_mutex());
        return (end_response(ret));
    }
    int     ret1 = send(_socket, buffer, ret, 0);
    if (!ret1 && ret1)
        _host->print(ERROR, "Error: Send response body interrupted.");
    if (ret1 == -1)
        _host->print(ERROR, "Error: Send body error.");
    if (ret1 <= 0)
        return (end_response(ret1));
    _worker->set_sk_timeout(_socket);
    _body_size += ret1;
    if (_content_length > 1000000)
        ft::print_loading_bar(_body_size, _content_length, 50, _host->get_cout_mutex());
    if (_body_size >= _content_length)
        return (end_response(ret1));
    return (ret1);
}

int     Response::end_response(int ret)
{
    if (_fd_out > 0)
    {
	_host->clear_read_fd(_fd_out);
        pthread_mutex_lock(_host->get_fd_mutex());
        close(_fd_out);
        _fd_out = -1;
        pthread_mutex_unlock(_host->get_fd_mutex());
    }
    pthread_mutex_lock(_host->get_cout_mutex());
    ft::timestamp();
    if (_status_code == 200)
        std::cout << GREEN;
    else if (_status_code > 300 && _status_code < 400)
        std::cout << MAGENTA;
    else
        std::cout << RED;
    std::cout << _request->get_location()->get_method_str(_request->get_method()) << " ";
    std::cout << _request->get_url() << " ";
    std::cout << _status_code << " ";
    std::cout << "Request: " << _request->get_body_size() << "b, ";
    std::cout << "Response: " << _body_size << "b ";
    std::cout << "[wk: " << _worker->get_id() << "] ";
    std::cout << "[sk: " << _socket << "]" << RESET << std::endl;
    pthread_mutex_unlock(_host->get_cout_mutex());
    _end = true;
    if (!_request->get_close())
    {
        init();
        _request->init();
    }
    return (ret);
}

int		        Response::get_status_code(void) const {return (_status_code);}
int             Response::get_fd_out(void) const {return (_fd_out);}
Host*		    Response::get_host(void) const {return (_host);}
Request*	    Response::get_request(void) const {return (_request);}
bool            Response::get_end_header(void) const {return (_end_header);}
size_t		    Response::get_content_length(void) const {return (_content_length);}
std::string	    Response::get_content_type(void) const {return (_content_type);}
std::string*	Response::get_header(void) {return (&_header);}
bool            Response::get_end(void) const {return (_end);}

void		Response::set_socket(int s) {_socket = s;}
void		Response::set_host(Host* h) {_host = h;}
void		Response::set_worker(Worker* w) {_worker = w;}
void		Response::set_server(Server* s) {_server = s;}
void		Response::set_request(Request* r) {_request = r;}
void		Response::set_status_code(int e) {_status_code = e;}
void        Response::set_fd_out(int f) {_fd_out = f;}
void		Response::set_content_length(size_t l) {_content_length = l;}
void	    Response::set_body(std::string s) {_body = s;}
void        Response::set_content_type(std::string t) {_content_type = t;}
