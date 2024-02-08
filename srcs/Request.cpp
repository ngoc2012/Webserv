/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nbechon <nbechon@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/02/08 07:25:08 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>

#include "Host.hpp"
#include "Address.hpp"
#include "Server.hpp"
#include "Location.hpp"
#include "Request.hpp"
#include "Header.hpp"
#include "Cgi.hpp"

#define BUFFER_SIZE 65792

Request::Request()
{
	//std::cout << "Request Default constructor" << std::endl;
}
Request::Request(const Request& src) { *this = src; }
Request&	Request::operator=( Request const & src )
{
	(void) src;
	return (*this);
}
Request::Request(int sk, Host* h, Address* a) : _socket(sk), _host(h), _address(a)
{
	//std::cout << "Request Constructor sk: " << sk << std::endl;
	_response.set_socket(sk);
	_response.set_host(h);
	_response.set_server(_server);
	_response.set_request(this);

	_header.set_request(this);
    _header.set_host(h);
    _header.set_str(&_str_header);

    _body_max = _host->get_client_max_body_size() * MEGABYTE;
    _body_buffer = _host->get_client_body_buffer_size() * KILOBYTE;
    _header_buffer = _host->get_large_client_header_buffer() * KILOBYTE;
    _buffer_size = MAX(_header_buffer, _body_buffer);
    _buffer = new char[_buffer_size];
    init();
}

void    Request::init(void)
{
    std::cout << "Request init" << std::endl;
    _server = 0;
    _cgi = 0;
	_str_header = "";
	_url = "";
	_method = NONE;
	_content_type = "";
	_content_length = 0;
	_chunked = false;
    _chunked_size = 0;
    _chunked_writed = 0;
    _body_left = 0;
    _header_size = 0;
	_body_size = 0;
    _session_id = "";
	_fd_in = -1;
	_full_file_name = "";
	_tmp_file = "";
    _end_header = false;
    _end = false;
    std::memset(_buffer, 0, _buffer_size);
	_status_code = 200;
}

Request::~Request()
{
    delete[] _buffer;
    if (_cgi)
        delete (_cgi);
    if (_socket > 0)
        close(_socket);
    if (_fd_in > 0)
        close(_fd_in);
    if (_tmp_file != "" && std::remove(_tmp_file.c_str()))
        std::cerr << MAGENTA << "Error: Can not delete file " << _tmp_file << RESET << std::endl;
}

int     Request::read(void)
{
    if (!_end_header)
        return (read_header());
    return (read_body());
}

int     Request::read_header()
{
    std::cout << "read_header" << std::endl;
    int ret = receive_header();
    if (ret <= 0)
        return (ret);
    if (!_end_header)
        return (1);
    if (!parse_header())
        return (end_request());
    if (_status_code != 200)
        return (end_request());
    process_fd_in();
    //std::cout << _body_size << " " << _body_left << " " << _content_length << std::endl;
    if (_fd_in != -1 && _body_left > 0)
    {
        if (_chunked)
            write_chunked(_body_left);
        else
        {
            // Écrire le contenu de _buffer[] dans le fichier associé à _fd_in
            ssize_t bytes_written = write(_fd_in, _buffer, _body_left);
            if (bytes_written == -1)
            {
                std::cerr << "Error: Unable to write to file " << _tmp_file << std::endl;
                _status_code = 500;
                end_request();
            }
            
        }
        _body_size += _body_left;
        std::memset(_buffer, 0, _buffer_size);
        _body_left = 0;
    }
    if (!_content_length)
        return (end_request());
    return (ret);
}

int Request::receive_header(void)
{
    size_t		    body_position;
    size_t		    received;
    int ret = 1;

    ret = recv(_socket, _buffer + _header_size, _header_buffer - _header_size, 0);
    std::cout << "receive_header:ret " << ret << std::endl;
    if (ret <= 0)
        return (ret);
    _host->set_sk_timeout(_socket);
    received = _header_size + ret;
    _buffer[received] = 0;
    
    /*
    std::cout << "The buffer:" << std::endl;
    char *c = _buffer;
    while (*c)
    {
        std::cout << (int) *c << ":";
        c++;
    }
    std::cout << std::endl;
    */

    _str_header = _buffer;
    body_position = _str_header.find("\r\n\r\n");
    if (body_position == NPOS)
    {
        _header_size += ret;
        if (_header_size > _header_buffer)
        {
            ft::timestamp();
            std::cerr << MAGENTA << "Error: No end header found or header too big.\n" << RESET << std::endl;
            _status_code = 400;	// Bad Request
            return (end_request());
        }
        return (1);
    }
    _header_size = body_position;
    _end_header = true;

    std::cout << "Request header: " << _str_header.size() << std::endl;
    std::cout << _str_header << std::endl;
    if (!_str_header.size())
    {
        std::cerr << MAGENTA << "Error: No header found." << RESET << std::endl;
        _status_code = 400;	// Bad Request
        return (end_request());
    }
    _body_left = received - _header_size - 4;
    std::cout << "_body_left: " << _body_left << " " << received << " " << _header_size << " " << body_position << " " << ret << std::endl;
    memmove(_buffer, _buffer + _header_size + 4, _body_left);
    /*
    _buffer[_body_left] = 0;
    std::cout << "The buffer:" << std::endl;
    char *c = _buffer;
    while (*c)
    {
        std::cout << (int) *c << ":";
        c++;
    }
    std::cout << std::endl;
    
    */
   return (ret);
}

bool	Request::parse_header(void)
{
    std::cout << "parse_header" << std::endl;
    if (!_header.parse_method_url(_url, _method))
    {
        _status_code = 400;	// Bad Request
        return (false);
    }
    ft::timestamp();
    std::cout << GREEN << _location->get_method_str(_method) << " ";
    std::cout << _url << " ";
    _host_name = _header.parse_host_name();
    if (_host_name == "")
    {
        _status_code = 400;	// Bad Request
        return (false);
    }
    std::cout << _host_name << RESET << std::endl;
    std::vector<Server*>        servers = _address->get_servers();
    std::vector<std::string>    server_names;
    _server = servers[0];
    for (std::vector<Server*>::iterator sv = servers.begin() + 1;
            sv != servers.end(); ++sv)
    {
        server_names = (*sv)->get_server_names();
        for (std::vector<std::string>::iterator it = server_names.begin() + 1;
                it != server_names.end(); ++it)
            if (_host_name == *it)
                _server = *sv;
    }
	_response.set_server(_server);
    if (!check_location())
        return (false);
    if (_location->get_redirection())
    {
        _status_code = _location->get_redirection();
        return (false);
    }
    if (_location->get_cgi_pass() != "")
    {
        _cgi = new Cgi(this);
        _cgi->set_pass(_location->get_cgi_pass());
        _cgi->set_file(_full_file_name);
    }
    _chunked = _header.parse_transfer_encoding();
    if (_chunked)
        return (true);
    _content_length = _header.parse_content_length();
    std::cout << "_chunked: " << _chunked << std::endl;
    //std::cout << "Content-Length: " << _content_length << std::endl;
    _cookies = _header.parse_cookies();
    if (_cookies.find("session_id") != _cookies.end())
        _session_id = _cookies["session_id"];
    if (_cookies.find("sid") != _cookies.end())
        _session_id = _cookies["sid"];
    //std::cout << "Session id: " << _session_id << std::endl;
    //std::cout << "Connection: " << _header.parse_connection() << std::endl;
    if (_header.parse_connection() == "close")
        _close = true;
    _content_type = _header.parse_content_type();
    //std::cout << "Content-Type: " << _content_type << std::endl;
    if (_content_length > _body_max)
    {
        _status_code = 400;	// Bad Request
        ft::timestamp();
        std::cerr << MAGENTA << "Error: Content length bigger than " << _body_max << RESET << std::endl;
        return (false);
    }
    return (true);
}

int     Request::read_body()
{
    int     ret;

    std::cout << "Read body start" << std::endl;
    ret = recv(_socket, _buffer + _body_left, _body_buffer - _body_left, 0);
    if (!ret)
        return (ret);
    if (ret < 0)
    {
        ft::timestamp();
        std::cerr << MAGENTA << "Error: recv error" << RESET << std::endl;
        _status_code = 400;
        return (end_request());
    }
	std::cout << "read_body: " << ret << std::endl;
    if (ret > 0)
        _host->set_sk_timeout(_socket);
    if (!_chunked)
    {
        _body_size += ret + _body_left;
        //std::cout << "_body_size: " << _body_size << std::endl;
        if (ret > 0 && write(_fd_in, _buffer, ret + _body_left) == -1)
            return (end_request());
    }
    else if (!write_chunked(ret + _body_left))
        return (end_request());
    //std::cout << "_body_size: " << _body_size << std::endl;
    //std::cout << "ret: " << ret << std::endl;
    //std::cout << "_content_length: " << _content_length << std::endl;
    if (_chunked && !_chunked_size)
        return (end_request());
    if ((!_content_length && ret < (int) _body_buffer)
            || (_content_length &&_body_size >= _content_length))
        return (end_request());
    return (ret);
}

bool    Request::write_chunked(size_t len)
{
    size_t		body_position = 0;

    std::cout << "write_chunked" << std::endl;
    _buffer[len] = 0;
    std::string     str_buffer(_buffer);
    std::cout << "_chunked_size: " << _chunked_size << " _chunked_writed: " << _chunked_writed << std::endl;
    if (_chunked_size - _chunked_writed > 0)
    {
        body_position = _chunked_size - _chunked_writed;
        if (body_position > len)
            body_position = len;
        _chunked_writed += body_position;
        _body_size += body_position;
        if (_body_size > _body_max)
        {
            _status_code = 400;
            std::cerr << "Error: Content length bigger than " << _body_max << std::endl;
            return (false);
        }
        if (write(_fd_in, _buffer, body_position) == -1)
        {
            std::cerr << "Error: Write fd in error 1." << std::endl;
            _status_code = 500;
            return (false);
        }
        //std::cout << "_body_size: " << _body_size << std::endl;
    }
    if (body_position > len - 1)
        return (true);
    //std::cout << "body_position: " << body_position << std::endl;
    size_t      pos = str_buffer.find("\r\n", body_position);
    //size_t      to_write;
    while (pos != NPOS)
    {
        _chunked_size = ft::atoi_base(str_buffer.substr(body_position, pos - body_position).c_str(), "0123456789abcdef");
        //std::cout << "_chunked_size: " << _chunked_size << std::endl;
        body_position = pos + 2;
        if (len < body_position)
            return (true);
        _chunked_writed = len - body_position;
        if (_chunked_writed > _chunked_size)
            _chunked_writed = _chunked_size;
        if (!_chunked_writed)
            return (true);
        _body_size += _chunked_writed;
        if (_body_size > _body_max)
        {
            _status_code = 400;
            std::cerr << "Error: Content length bigger than " << _body_max << std::endl;
            return (false);
        }
        if (write(_fd_in, &_buffer[body_position], _chunked_writed) == -1)
        {
            std::cerr << "Error: Write fd in error 2." << std::endl;
            _status_code = 500;
            return (false);
        }
        //std::cout << "_body_size: " << _body_size << std::endl;
        body_position += _chunked_writed;
        pos = str_buffer.find("\r\n", body_position);
    }
    _body_left = len - body_position;
    memcpy(_buffer, _buffer + body_position, _body_left);
    return (true);
}

bool	Request::check_location()
{
    _location = Location::find_location(_url,
            _server->get_locations(), _method, _status_code);
    if (!_location || _status_code != 200)
        return (false);
    if (_location->get_redirection())
        return (true);
    _full_file_name = _location->get_full_file_name(_url,
            _server->get_root(), _method);
    std::cout << _full_file_name << std::endl;
    struct stat info;
    if (_method != PUT
            && stat(_full_file_name.c_str(), &info) != 0)
    {
        _status_code = 404; // Not found
        return (false);             
    }
    return (true);
}

bool	Request::check_session()
{
    return (true);
}

void	Request::process_fd_in()
{
    //std::cout << "process_fd_in" << std::endl;
    int i = 0;
    switch (_method)
    {
		case OPTIONS:
        case HEAD:
        case GET:
            break;
        case PUT:
            _fd_in = open(_full_file_name.c_str(),
                    O_CREAT | O_WRONLY | O_TRUNC, 0664);
            if (_fd_in == -1)
            {
                std::cerr << "Error: PUT method, fd in open error." << std::endl;
                _status_code = 500;
            }
            break;
        case POST:
            _tmp_file = "/tmp/0";
            struct stat buffer;
            while (stat(_tmp_file.c_str(), &buffer) == 0)
                _tmp_file = "/tmp/" + ft::itos(++i);
            _fd_in = open(_tmp_file.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0664);
            if (_fd_in == -1)
            {
                std::cerr << "Error: POST method, fd in open error." << std::endl;
                _status_code = 500;
                end_request();
            }
            break;
        case DELETE:
            if (std::remove(_full_file_name.c_str()))
                std::cerr << "Error: Can not delete file " << _tmp_file << std::endl;
            break;
        case NONE:
            break;
    }
    //std::cout << "end process_fd_in" << std::endl;
}

int     Request::end_request(void)
{
    std::cout << "end_request: socket=" << _socket << " body size=" << _body_size << " file name=" << _full_file_name << std::endl;
    if (!_cgi && _fd_in > 0)
        close(_fd_in);
    _host->new_response_sk(_socket);
    _response.set_status_code(_status_code);
    if (_status_code == 200 && _cgi)
        _status_code = _cgi->execute();
    _end = true;
    //if (_status_code == -1)
    //{
    //    delete _host;
    //    exit(1);
    //}
    return (1);
}

Host*		    Request::get_host(void) const {return (_host);}
Server*		    Request::get_server(void) const {return (_server);}
e_method	    Request::get_method(void) const {return (_method);}
std::string	    Request::get_url(void) const {return (_url);}
Response*	    Request::get_response(void) {return (&_response);}
Cgi*            Request::get_cgi(void) const {return (_cgi);}
int		        Request::get_status_code(void) const {return (_status_code);}
std::string	    Request::get_content_type(void) const {return (_content_type);}
size_t		    Request::get_content_length(void) const {return (_content_length);}
size_t		    Request::get_body_size(void) const {return (_body_size);}
std::string	    Request::get_str_header(void) const {return (_str_header);}
std::string	    Request::get_full_file_name(void) const {return (_full_file_name);}
Location*	    Request::get_location(void) const {return (_location);}
int		        Request::get_fd_in(void) const {return (_fd_in);}
std::string	    Request::get_session_id(void) const {return (_session_id);}
bool		    Request::get_end(void) const {return (_end);}

void		    Request::set_fd_in(int f) {_fd_in = f;}
//void		    Request::set_end(bool e) {_end = e;}
