/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nbechon <nbechon@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/02/22 11:42:04 by ngoc             ###   ########.fr       */
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
//#include "Cgi.hpp"

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
	_accept_encoding = "";
	_chunked = false;
    _body_left = 0;
    _header_size = 0;
	_body_size = 0;
    _session_id = "";
	_fd_in = -1;
	_full_file_name = "";
	_tmp_file = "";
    _end_header = false;
    //_end_chunked_body = false;
    _start_chunked_body = false;
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
            write_chunked();
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
    if (!_chunked && (!_content_length || _body_size >= _content_length))
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
    std::cout << "`" << _str_header.substr(0, 200) << "`" << std::endl;
    _header_size = _str_header.find("\r\n\r\n");
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
    std::cout << "Body left: " << _body_left << std::endl;

    if (_body_left > 0)
    {
        std::memmove(_buffer, body_left.c_str(), _body_left);
        _buffer[_body_left] = 0;
        //std::cout << "Body: `" << _buffer << "`" << std::endl;
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

    //std::cout << "Read body start" << std::endl;
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
	//std::cout << "read_body: " << ret << std::endl;
    _buffer[ret] = 0;
    _host->set_sk_timeout(_socket);
    if (_chunked)
    {
        write_chunked();
        return (ret);
    }
    //std::cout << "_body_size: " << _body_size << std::endl;
    _body_size += ret;
    if (ret > 0 && write(_fd_in, _buffer, ret) == -1)
        return (end_request());
    if (_body_size >= _content_length)
        return (end_request());
    return (ret);
}

static size_t   find_chunk_size(std::string &s, size_t &_chunk_size)
{
    size_t  start;
    size_t  end;
    size_t  size;
    
    start = s.find("\r\n");
    if (start == NPOS)
        return (NPOS);
    start += 2;
    end = s.find("\r\n", start);
    if (end == NPOS)
        return (NPOS);
    //std::cout << "size:'" << s.substr(start, end - start) << "'" << std::endl;
    size = ft::atoi_base(s.substr(start, end - start).c_str(), "0123456789abcdef");
    if (size == 0 && s.find("\r\n\r\n", start) == NPOS)
        return (NPOS);
    _chunk_size = size;
    return (end + 2);
}

void    Request::write_chunked()
{
    size_t          end_size;
    size_t          len;
    size_t          read_size;

    //std::cout << "write_chunked" << _buffer << std::endl;
    if (!_start_chunked_body)
    {
        _read_data += "\r\n";
        _start_chunked_body = true;
    }
    _read_data += std::string(_buffer);
    read_size = _read_data.size();
    if (!read_size)
    {
        std::cerr << RED << "Error: No chunked data received." << RESET << std::endl;
        return ;
    }
    do
    {
        //std::cout << "_chunked_data: (" << read_size << "," << _chunk_size << "), body_size = " << _body_size << std::endl;
        //std::cout << " `" << _read_data.substr(0, 100) << "`" << std::endl;
        if (_chunk_size > 0)
        {
            len = _chunk_size;
            if (read_size < len)
                len = read_size;
            if (write(_fd_in, _read_data.c_str(), len) == -1)
            {
                std::cerr << RED << "Error: Chunked data: Write fd in error(1)." << RESET << std::endl;
                _status_code = 500;
                return ;
            }
            _body_size += len;
            _chunk_size -= len;
            read_size -= len;
            _read_data.erase(0, len);
            if (!read_size)
                return ;
        }
        end_size = find_chunk_size(_read_data, _chunk_size);
        if (end_size == NPOS)
            return ;
        if (!_chunk_size)
        {
            std::cout << "End chunked body" << std::endl;
            end_request();
            return ;
        }
        _read_data.erase(0, end_size);
        //std::cout << "After erase: `" << _read_data.substr(0, 100) << "`" << std::endl;
        read_size = _read_data.size();
    } while (read_size > 0);
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
    //std::cout << "Cgi: " << _location->get_cgi_pass() << std::endl;
    if (_location->get_cgi_pass() != "")
    {
        _cgi = new Cgi(this);
        _cgi->set_pass(_location->get_cgi_pass());
        _cgi->set_file(_full_file_name);
    }

    struct stat info;
    if (!_cgi && _method != PUT
            && stat(_full_file_name.c_str(), &info) != 0)
    {
        std::cerr << RED << "Error: File does not exist." << RESET << std::endl;
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
bool		    Request::get_chunked(void) const {return (_chunked);}
std::string	    Request::get_accept_encoding(void) const {return (_accept_encoding);}

void		    Request::set_fd_in(int f) {_fd_in = f;}
void            Request::set_accept_encoding(std::string a) {_accept_encoding = a;}
//void		    Request::set_end(bool e) {_end = e;}
