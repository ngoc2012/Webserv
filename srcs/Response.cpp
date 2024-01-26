/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/01/10 10:17:27 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <time.h>

#include <sys/types.h>
#include <sys/wait.h>

#include "Host.hpp"
#include "Server.hpp"
#include "Location.hpp"
#include "Request.hpp"
#include "Header.hpp"
#include "Cgi.hpp"
#include "Listing.hpp"

#include "Response.hpp"

Response::Response()
{
    //std::cout << "Response Default constructor" << std::endl;
    _header = "";
    _body = "";
    _status_code = 200;
    _content_length = 0;
    _body_size = 0;
    _pos = 0;

    _full_file_name = "";
    _write_queue = false;
    _fd_out = -1;
}
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

int     Response::write()
{
    if (!_write_queue)
        return (0);
    if (_header == "")
        write_header();
    else
        write_body();
    return (0);
}

void     Response::write_header()
{
    //std::cout << "write_header" << std::endl;

    _full_file_name = _request->get_full_file_name();
    //std::cout << "write_header " << _full_file_name << std::endl;
    Header	header(this, ft::file_extension(_full_file_name));
    if (_status_code == 200)
    {
        header.set_allow(_request->get_location()->get_methods_str());
        if (_request->get_method() == GET)
            get_file_size();
    }
    if (_status_code != 200)
    {
        std::string mess = (*_host->get_status_message())[_status_code];
        mess_body(ft::itos(_status_code) + " " + mess, mess);
    }
    else if (_request->get_method() == DELETE)
        mess_body("Delete", "File deleted");
    std::string     sid = _request->get_session_id();
    if (sid != "")
    {
        Sessions*       sessions = _server->get_sessions();
        if (!sessions->verify(sid))
        {
            std::string token = sessions->create_token();
            sessions->add(token, std::time(0) + 30);
            header.set_session_id(token);
        }
    }
    _header = header.generate();
    //std::cout << "Response Header:\n" << _header << std::endl;
    if (send(_socket, _header.c_str(), _header.length(), 0) < 0)
        end_connection();
    //else
    //    std::cout << "Header sent" << std::endl;
}

void     Response::mess_body(std::string title, std::string body)
{
    _body += "<!DOCTYPE html>\n";
    _body += "    <html>\n";
    _body += "    <head>\n";
    _body += "    <title>" + title + "</title>\n";
    _body += "    </head>\n";
    _body += "    <body>\n";
    _body += "    <h1>" + body + "</h1>\n";
    //_body += "    <p>The requested URL /example-page was not found on this server.</p>\n";
    _body += "    </body>\n";
    _body += "    </html>\n";
    //_body = (*_host->get_status_message())[_status_code];
    _content_length = _body.size();
    //std::cout << "error_body " << _body << std::endl;
}

void     Response::get_file_size()
{
    struct stat fileStat;
    if (stat(_full_file_name.c_str(), &fileStat) != 0)
    {

        std::cerr << "Error: File or folder not found." << std::endl;
        _status_code = 500;
        return ;
    }
    if (S_ISDIR(fileStat.st_mode))
    {
        _body = Listing::get_html(this);
        _content_length = _body.size();
        return ;
    }
    _fd_out = open(_full_file_name.c_str(), O_RDONLY);
    if (_fd_out == -1)
    {
        _status_code = 500;
        return ;
    }
    _content_length = fileStat.st_size;
    //std::cout << "get_file_size _content_length: " << _content_length << std::endl;
}

int     Response::write_body()
{
    if (_body != "")
    {
        size_t     len = _content_length - _pos;
        if (len > RESPONSE_BUFFER * 1028)
            len = RESPONSE_BUFFER * 1028;

        //std::cout << "write_body " << _pos << " " << len << " " << _body << std::endl;
        if (send(_socket, &_body.c_str()[_pos], len, 0) < 0)
            return (end_connection());

        _pos += len;
        if (_pos >= _content_length)
            return (end_connection());
        return (0);
    }
    if (_fd_out == -1)
        return (end_connection());
    char	buffer[RESPONSE_BUFFER * 1028];
    int ret = read(_fd_out, buffer, RESPONSE_BUFFER * 1028);

    if (ret <= 0)
        return (end_connection());

    _body_size += ret;
    if (send(_socket, buffer, ret, 0) < 0)
        return (end_connection());
    return (0);
}

int     Response::end_connection(void)
{
    if (_request->get_method() == POST
        && _request->get_cgi()->get_pid() != -1)
        waitpid(_request->get_cgi()->get_pid(), NULL, 0);
    std::cout << "end_connection " << _socket << " " << _full_file_name << std::endl;
    if (_fd_out > 0)
        close(_fd_out);
    _write_queue = false;
    _host->close_client_sk(_socket);
    return (0);
}

int		    Response::get_status_code(void) const {return (_status_code);}
size_t		Response::get_content_length(void) const {return (_content_length);}
int         Response::get_fd_out(void) const {return (_fd_out);}
Host*		Response::get_host(void) const {return (_host);}
Request*	Response::get_request(void) const {return (_request);}

void		Response::set_socket(int s) {_socket = s;}
void		Response::set_host(Host* h) {_host = h;}
void		Response::set_server(Server* s) {_server = s;}
void		Response::set_request(Request* r) {_request = r;}
void		Response::set_status_code(int e) {_status_code = e;}
void        Response::set_write_queue(bool b) {_write_queue = b;}
void        Response::set_fd_out(int f) {_fd_out = f;}
