/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nbechon <nbechon@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/02/21 11:13:05 by minh-ngu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <cstring>  // debug

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
    _chunk_size = 0;
    std::cout << "_body_buffer: " << _body_buffer << std::endl;
    std::cout << "_header_buffer: " << _header_buffer << std::endl;
    std::cout << "_buffer_size: " << _buffer_size << std::endl;
    _buffer = new char[_buffer_size + 1];

    _cgi = 0;
    _fd_in = -1;
    _tmp_file = "";
    init();
}

void    Request::clean(void)
{
    if (_cgi)
        delete (_cgi);
    if (_fd_in > 0)
        close(_fd_in);
    if (_tmp_file != "" && std::remove(_tmp_file.c_str()))
        std::cerr << MAGENTA << "Error: Can not delete file " << _tmp_file << RESET << std::endl;
}

void    Request::init(void)
{
    std::cout << "Request init" << std::endl;
    clean();
    _server = 0;
    _cgi = 0;
	_str_header = "";
	_url = "";
	_method = NONE;
	_content_type = "";
	_content_length = 0;
	_chunked = false;
    _body_left = 0;
    _header_size = 0;
	_body_size = 0;
    _session_id = "";
	_fd_in = -1;
	_full_file_name = "";
	_tmp_file = "";
    _end_header = false;
    _end_chunked_body = true;
    _read_data = "";
    _end = false;
    std::memset(_buffer, 0, _buffer_size + 1);
	_status_code = 200;
}

Request::~Request()
{
    //std::cout << "Request Destructor" << std::endl;
    delete[] _buffer;
    clean();
    if (_socket > 0)
        close(_socket);
}

int     Request::read(void)
{
    if (!_end_header)
        return (read_header());
    return (read_body());
}

int     Request::read_header()
{
    int ret = receive_header();
    std::cout << "read_header: " << ret << std::endl;
    if (ret <= 0)
        return (ret);
    if (!_end_header)
        return (1);
    if (!parse_header())
        return (end_request());
    process_fd_in();
    std::cout << "_fd_in = " << _fd_in << ", _body_left = " << _body_left << std::endl;
    //std::cout << _body_size << " " << _body_left << " " << _content_length << std::endl;
    if (_fd_in != -1 && _body_left > 0)
    {
        std::cout << "Write body left to fd_in, body_left=" << _body_left << std::endl;
        if (_chunked)
            write_chunked(true);
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
            _body_size = _body_left;
        }
        std::memset(_buffer, 0, _body_left);
    }
    if ((!_chunked && (!_content_length || _body_left >= _content_length)) || (_chunked && _end_chunked_body))
    {
        std::cout << "No content length or the header receive has all the data body_left=" << _body_left << ", content_length=" << _content_length << std::endl;
        return (end_request());
    }
        
    return (ret);
}

int Request::receive_header(void)
{
    int ret = recv(_socket, _buffer, _header_buffer, 0);
    std::cout << "receive_header: ret=" << ret << std::endl;
    if (ret < 0)
        return (ret);
    if (!ret)
    {
        if (_str_header.size())
        {
            std::cerr << MAGENTA << "Error: No end header found." << RESET << std::endl;
            _status_code = 400;	// Bad Request
            return (end_request());
        }
        else
            return (ret);
    }
    _host->set_sk_timeout(_socket);
    _buffer[ret] = 0;
    _str_header += _buffer;
    std::cout << "============================================" << std::endl;
    std::cout << "`" << _str_header << "`" << std::endl;
    _header_size = _str_header.find("\r\n\r\n");
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
    if (_header_size == NPOS)
    {
        if (_str_header.size() > _header_buffer)
        {
            ft::timestamp();
            std::cerr << MAGENTA << "Error: No end header found or header too big.\n" << RESET << std::endl;
            _status_code = 400;	// Bad Request
            return (end_request());
        }
        return (ret);
    }
    _end_header = true;
    std::cout << "Request header: " << _header_size << std::endl;
    _body_left = _str_header.size() - _header_size - 4;
    std::string body_left = _str_header.substr(_header_size + 4);
    if (_body_left > 0)
    {
        std::cout << "Body left: " << _body_left << std::endl;
        std::memmove(_buffer, body_left.c_str(), _body_left);
        _buffer[_body_left] = 0;
        std::cout << "Body: `" << _buffer << "`" << std::endl;
    }
    
    std::cout << "End receive header with status code " << _status_code << std::endl;
    return (ret);
}

bool	Request::parse_header(void)
{
    std::cout << "parse_header" << std::endl;
    if (!_header.parse_method_url(_url, _method))
    {
        ft::timestamp();
        std::cerr << RED << "Error: Method or url error." << RESET << std::endl;
        _status_code = 400;	// Bad Request
        return (false);
    }
    ft::timestamp();
    std::cout << GREEN << _location->get_method_str(_method) << " ";
    std::cout << _url << " ";
    _host_name = _header.parse_host_name();
    if (_host_name == "")
    {
        ft::timestamp();
        std::cerr << RED << "Error: No host name." << RESET << std::endl;
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
    check_location();
    std::cout << "Location found: " << _location->get_url() << std::endl;
    _chunked = _header.parse_transfer_encoding();
    std::cout << "_chunked: " << _chunked << std::endl;
    if (_chunked)
        _end_chunked_body = false;
    _content_length = _header.parse_content_length();
    //std::cout << "Content-Length: " << _content_length << std::endl;
    _cookies = _header.parse_cookies();
    if (_cookies.find("session_id") != _cookies.end())
        _session_id = _cookies["session_id"];
    if (_cookies.find("sid") != _cookies.end())
        _session_id = _cookies["sid"];
    std::cout << "Session id: " << _session_id << std::endl;
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
    std::cout << "End parse_header with status code " << _status_code << std::endl;
    return (true);
}

int     Request::read_body()
{
    int     ret;

    std::cout << "Read body start" << std::endl;
    ret = recv(_socket, _buffer, _body_buffer, 0);
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
    _buffer[ret] = 0;
    _host->set_sk_timeout(_socket);
    if (_chunked)
        write_chunked(true);
    else
    {
        //std::cout << "_body_size: " << _body_size << std::endl;
        _body_size += ret;
        if (ret > 0 && write(_fd_in, _buffer, ret) == -1)
            return (end_request());
    }
    //std::cout << "_body_size: " << _body_size << std::endl;
    //std::cout << "ret: " << ret << std::endl;
    //std::cout << "_content_length: " << _content_length << std::endl;
    if ((_chunked && _end_chunked_body) || (!_chunked && _body_size >= _content_length))
        return (end_request());
    return (ret);
}

struct  chunk_size_s
{
    size_t  start;
    size_t  end;
    size_t  value;
};

static void find_chunk_size(std::string &s, chunk_size_s &cs)
{
    cs.value = NPOS;
    cs.start = s.find("\r\n");
    if (cs.start == NPOS)
        return ;
    cs.end = s.find("\r\n", cs.start + 2);
    if (cs.end == NPOS)
        return ;
    cs.value = ft::atoi_base(s.substr(cs.start, cs.end - cs.start - 2).c_str(), "0123456789abcdef");
    cs.end += 2;
    return ;
}

bool    Request::write_chunked(bool read_buffer)
{
    chunk_size_s    cs;
    size_t          len;
    size_t          read_size;

    //std::cout << "write_chunked" << _buffer << std::endl;
    if (read_buffer)
        _read_data += std::string(_buffer);
    std::cout << "_chunked_data: (" << _read_data.size() << "," << _chunk_size << ")" << std::endl;
    std::cout << " `" << _read_data.substr(0, 100) << "`" << std::endl;
    read_size = _read_data.size();
    if (!read_size)
    {
        std::cerr << RED << "Error: No chunked data received." << RESET << std::endl;
        _end_chunked_body = true;
        return (true);
    }
    if (_chunk_size > 0)
    {
        len = _chunk_size;
        if (read_size <= len)
            len = read_size;
        if (write(_fd_in, _read_data.c_str(), len) == -1)
        {
            std::cerr << RED << "Error: Chunked data: Write fd in error(1)." << RESET << std::endl;
            _status_code = 500;
            return (false);
        }
        _body_size += len;
        _chunk_size -= len;
        _read_data.erase(0, len);
        return (true);
    }
    find_chunk_size(_read_data, cs);
    if (cs.value == NPOS)
        return (true);
    _chunk_size = cs.value;
    while (cs.value != NPOS)
    {
        if (_chunk_size <= 0)
        {
            _chunk_size = ft::atoi_base(_chunked_data.substr(start_size, end_size - start_size).c_str(), "0123456789abcdef");
            std::cout << "chunk_size: " << _chunked_data.substr(start_size, end_size - start_size) <<   ":" << _chunk_size << std::endl;
            data_position = end_size + 2;
            std::cout << "'" << _chunked_data.substr(start_size, 100) << "'" << std::endl; 
            std::cout << "data_position: " << data_position << "," << _chunked_data.size() << std::endl;
            if (data_position >= _chunked_data.size())
            {
                std::cout << "data_position >=  _chunked_data.size()" << std::endl;
                _chunked_data = _chunked_data.substr(start_size);
                return (true);
            }
        }
        //std::cout << _chunked_data.substr(data_position) << std::endl;
        read_chunk = _chunked_data.find("\r\n", data_position);
        if (read_chunk == NPOS)
        {
            len = _chunked_data.size() - data_position;
            if (write(_fd_in, &_chunked_data.c_str()[data_position], len) == -1)
            {
                std::cerr << "Error: Write fd in error 2." << std::endl;
                _status_code = 500;
                return (false);
            }
            _body_size += len;
            _chunk_size -= len;
            std::cout << "body_size: " << _body_size << ", _chunk_size = " << _chunk_size << std::endl;
            _chunked_data = "";
            return (true);
        }
        start_size = read_chunk + 2;
        read_chunk -= data_position;
        if (!_chunk_size || !read_chunk)
        {
            std::cout << "chunk_size == 0 || read_chunk == 0" << std::endl;
            _end_chunked_body = true;
            break;
        }
        _body_size += read_chunk;
        if (_body_size > _body_max)
        {
            _status_code = 400;
            std::cerr << RED << "Error: Content length bigger than body_max: " << _body_max << RESET << std::endl;
            return (false);
        }
        if (write(_fd_in, &_chunked_data.c_str()[data_position], read_chunk) == -1)
        {
            std::cerr << "Error: Write fd in error 2." << std::endl;
            _status_code = 500;
            return (false);
        }
        _chunk_size = 0;
        _chunked_data = _chunked_data.substr(start_size);
        start_size = 0;
        if (_content_length && _body_size >= _content_length)
            break;
        //std::cout << "start_size: " << start_size << std::endl;
        end_size = _chunked_data.find("\r\n", start_size);
    }
    std::cout << "body_size: " << _body_size << std::endl;
    if (_chunked_data.find("0\r\n\r\n") != NPOS || (_content_length && _body_size >= _content_length))
    {
        std::cout << "_end_chunked_body" << (_chunked_data.find("\r\n0\r\n\r\n") == NPOS) << ":";
        std::cout << _body_size << ":" << _content_length << std::endl;
        _end_chunked_body = true;
    }
    return (true);
}

bool	Request::check_location()
{
    _location = Location::find_location(_url,
            _server->get_locations(), _method, _status_code);
    if (!_location || _status_code != 200)
        return (false);
    if (_location->get_redirection())
    {
        _status_code = _location->get_redirection();
        return (true);
    }
    _full_file_name = _location->get_full_file_name(_url,
            _server->get_root(), _method);
    std::cout << _full_file_name << std::endl;
    struct stat info;
    if (_method != PUT
            && stat(_full_file_name.c_str(), &info) != 0)
    {
        std::cerr << RED << "Error: File does not exist." << RESET << std::endl;
        _status_code = 404; // Not found
        return (false);             
    }
    if (_location->get_cgi_pass() != "")
    {
        _cgi = new Cgi(this);
        _cgi->set_pass(_location->get_cgi_pass());
        _cgi->set_file(_full_file_name);
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
    
    if (_status_code == 200 && _cgi)
        _status_code = _cgi->execute();
    if (_fd_in > 0)
        close(_fd_in);
    _host->new_response_sk(_socket);
    _response.set_status_code(_status_code);
    _end = true;
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
