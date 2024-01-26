/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nbechon <nbechon@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/01/26 11:53:26 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Host.hpp"
#include "Address.hpp"
#include "Server.hpp"
#include "Location.hpp"
#include "Request.hpp"
#include "Header.hpp"
#include "Cgi.hpp"

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

    _server = 0;
	_response.set_socket(sk);
	_response.set_host(h);
	_response.set_server(_server);
	_response.set_request(this);
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
	_body_size = 0;
    _header.set_host(h);
    _header.set_str(&_str_header);
    _session_id = "";

	_fd_in = -1;
	_full_file_name = "";
    _body_max = _host->get_client_max_body_size() * MEGABYTE;
    _body_buffer = _host->get_client_body_buffer_size() * KILOBYTE;
    _buffer = new char[_body_buffer * 2 + 1];
	_read_queue = true;
	_tmp_file = "";

	_status_code = 200;
}

Request::~Request()
{
    delete[] _buffer;
    if (_cgi)
        delete (_cgi);
    if (_socket > 0)
        close(_socket);
    if (_tmp_file != "" && std::remove(_tmp_file.c_str()))
        std::cerr << "Error: Can not delete file " << _tmp_file << std::endl;
}

int     Request::read(void)
{
    if (!_read_queue)
        return (0);
    if (_str_header == "")
        read_header();
    else
        read_body();
    return (0);
}

int     Request::read_header()
{
    //std::cout << "read_header _body_size: " << _body_size << std::endl;
    if (!receive_header())
        return (end_read());
    if (!parse_header())
        return (end_read());
    process_fd_in();
    if (_status_code != 200)
        return (end_read());
    return (0);
}

bool	Request::receive_header(void)
{
    std::string     str_buffer;
    size_t		    body_position;
    int ret = 1;

    body_position = NPOS;
    while (body_position == NPOS && ret > 0)
    {
        ret = recv(_socket, _buffer, _body_buffer, 0);
        //std::cout << "ret " << ret << std::endl;
        if (ret < 0)
        {
            _status_code = 500;
            return (false);
        }
        if (ret > 0)
        {
            _buffer[ret] = 0;
            str_buffer = _buffer;
            _str_header += str_buffer;
            body_position = str_buffer.find("\r\n\r\n");
        }
    }
    std::cout << "Request header: " << _str_header.size() << std::endl << _str_header << std::endl;
    if (body_position == NPOS)
    {
        std::cerr << "Error: No end header found.\n" << std::endl;
        _status_code = 400;	// Bad Request
        return (false);
    }
    body_position += 4;
    _body_left = ret - body_position;
    //std::cout << "_body_left: " << _body_left << " " << body_position << " " << ret << std::endl;
    memcpy(_buffer, _buffer + body_position, _body_left);
    return (true);
}

bool	Request::parse_header(void)
{
    if (!_header.parse_method_url(_url, _method))
    {
        _status_code = 400;	// Bad Request
        return (false);
    }
    _host_name = _header.parse_host_name();
    if (_host_name == "")
    {
        _status_code = 400;	// Bad Request
        return (false);
    }
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
    if (!check_location())
        return (false);
    if (_location->get_cgi_pass() != "")
    {
        _cgi = new Cgi(this);
        _cgi->set_pass(_location->get_cgi_pass());
        _cgi->set_file(_full_file_name);
    }
    _chunked = _header.parse_transfer_encoding();
    //std::cout << "_chunked: " << _chunked << std::endl;
    if (_chunked)
        return (true);
    _content_length = _header.parse_content_length();
    std::cout << "Content-Length: " << _content_length << std::endl;
    if (_method == GET)
    {
        if (_content_length == NPOS)
            return (false);
        return (true);
    }
    _content_type = _header.parse_content_type();
    std::cout << "Content-Type: " << _content_type << std::endl;
    if (_method == PUT && _content_length != NPOS)
        return (true);
    if (_method == POST)
        return (true);
    if (_content_type == "" || _content_length == NPOS)
    {
        _status_code = 400;	// Bad Request
        return (false);
    }
    if (_content_length > _body_max)
    {
        _status_code = 400;	// Bad Request
        std::cerr << "Error: Content length bigger than " << _body_max << std::endl;
        return (false);
    }
    _cookies = _header.parse_cookies();
    if (_cookies.find("sesion_id") != _cookies.end())
        _session_id = _cookies["session_id"];
    if (_cookies.find("sid") != _cookies.end())
        _session_id = _cookies["sid"];
    std::cout << "Session id: " << _session_id << std::endl;
    return (true);
}

int     Request::read_body()
{
    int     ret;

    ret = recv(_socket, _buffer + _body_left, _body_buffer, 0);
    if (ret < 0)
    {
        std::cerr << "Error: recv error" << std::endl;
        _status_code = 400;
        return (end_read());
    }
	//std::cout << "read_body: " << ret << std::endl;
    if (!_chunked)
    {
        _body_size += ret + _body_left;
        //std::cout << "_body_size: " << _body_size << std::endl;
        if (ret > 0 && write(_fd_in, _buffer, ret + _body_left) == -1)
            return (end_read());
        _body_left = 0;
    }
    else if (!write_chunked(ret + _body_left))
        return (end_read());
    //std::cout << "_body_size: " << _body_size << std::endl;
    //std::cout << "ret: " << ret << std::endl;
    //std::cout << "_content_length: " << _content_length << std::endl;
    if (_chunked && !_chunked_size)
        return (end_read());
    if ((!_content_length && ret < (int) _body_buffer)
            || (_content_length &&_body_size >= _content_length))
        return (end_read());
    return (0);
}

bool    Request::write_chunked(size_t len)
{
    size_t		body_position = 0;

    _buffer[len] = 0;
    std::string     str_buffer(_buffer);
    //std::cout << "_chunked_size: " << _chunked_size << " _chunked_writed: " << _chunked_writed << std::endl;
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
    //std::cout << "============================" << std::endl;
    //std::cout << "Header:" << _header.size() << std::endl  << _header << std::endl;
    //std::cout << "============================" << std::endl;
    _location = Location::find_location(_url,
            _server->get_locations(), _method, _status_code);

    if (!_location)
        return (false);
    // if (!_location || _status_code != 200)
    //     return (false);

    _full_file_name = _location->get_full_file_name(_url,
            _server->get_root(), _method);
    //std::cout << "check_location " << _socket << " " << _full_file_name << std::endl;

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
    std::cout << "process_fd_in" << std::endl;
    int i = 0;
    switch (_method)
    {
        case GET:
            break;
        case PUT:
            _fd_in = open(_full_file_name.c_str(),
                    O_CREAT | O_WRONLY | O_TRUNC, 0664);
            if (_fd_in == -1)
                _status_code = 500;
            break;
        case POST:
            _tmp_file = "/tmp/0";
            struct stat buffer;
            while (stat(_tmp_file.c_str(), &buffer) == 0)
                _tmp_file = "/tmp/" + ft::itos(++i);

            _fd_in = open(_tmp_file.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0664);
            if (_fd_in == -1)
            {
                _status_code = 500;
                end_read();
            }
            else
            {
                if (!_content_length || _content_length == NPOS)
                {
                    end_read();
                }
            else
            {
                // Écrire le contenu de _buffer[] dans le fichier associé à _fd_in
                ssize_t bytes_written = write(_fd_in, _buffer, _content_length);
                if (bytes_written == -1)
                {
                    std::cerr << "Error: Unable to write to file " << _tmp_file << std::endl;
                    _status_code = 500;
                    end_read();
                }
            }
        }
        break;
        case DELETE:
            if (std::remove(_full_file_name.c_str()))
                std::cerr << "Error: Can not delete file " << _tmp_file << std::endl;
            break;
        case NONE:
            break;
    }
}

int     Request::end_read(void)
{
    std::cout << "end_read " << _socket << " " << _body_size << " " << _full_file_name << std::endl;

    if (!_cgi && _fd_in > 0)
        close(_fd_in);
    _read_queue = false;
    _host->new_response_sk(_socket);
    _response.set_status_code(_status_code);
    if (_status_code == 200 && _cgi)
        _cgi->execute();
    _response.set_write_queue(true);
    return (0);
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

void		    Request::set_fd_in(int f) {_fd_in = f;}
