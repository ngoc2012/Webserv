/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nbechon <nbechon@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/03/05 16:51:02 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <cstring>  // debug
#include <sys/stat.h>	// stat, S_ISDIR
#include <cerrno> // For errno
#include <cstring> // For strerror

#include "Host.hpp"
#include "Worker.hpp"
#include "Address.hpp"
#include "Server.hpp"
#include "Location.hpp"
#include "Request.hpp"
#include "Header.hpp"
#include "Listing.hpp"


#define BUFFER_SIZE 65792

Request::Request(){}

Request::Request(const Request& src) { *this = src; }

Request&	Request::operator=( Request const & src )
{
	(void) src;
	return (*this);
}
Request::Request(int sk, Worker* w, Address* a) : _socket(sk), _worker(w), _address(a)
{
    _host = _worker->get_host();

	_response.set_socket(sk);
	_response.set_host(_host);
    _response.set_worker(_worker);
	_response.set_server(_server);
	_response.set_request(this);
    
    _body_buffer = _host->get_client_body_buffer_size() * KILOBYTE;
    _header_buffer = _host->get_large_client_header_buffer() * KILOBYTE;
    _buffer_size = MAX(_header_buffer, _body_buffer);
    _chunk_size = 0;
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
    {
        // pthread_mutex_lock(_host->get_fd_mutex());
        close(_fd_in);
        // pthread_mutex_unlock(_host->get_fd_mutex());
        _fd_in = -1;
    }
    if (_tmp_file != "" && std::remove(_tmp_file.c_str()))
    {
        pthread_mutex_lock(_host->get_cout_mutex());
        std::cerr << MAGENTA << "Error: Can not delete file " << _tmp_file << RESET << std::endl;
        pthread_mutex_unlock(_host->get_cout_mutex());
    }
}

void    Request::init(void)
{
    clean();
    _body_max = _host->get_client_max_body_size() * MEGABYTE;
    _server = 0;
    _cgi = 0;
	_str_header = "";
	_url = "";
	_method = NONE;
	_content_type = "";
	_content_length = 0;
	_chunked = false;
    _body_left = 0;
	_body_size = 0;
    _session_id = "";
	_fd_in = -1;
	_full_file_name = "";
	_tmp_file = "";
    _end_header = false;
    _start_chunked_body = false;
    _read_data = "";
    _end = false;
    _close = false;
    _fields.clear();
    std::memset(_buffer, 0, _buffer_size + 1);
	_status_code = 200;
    _timeout = _host->get_timeout();
}

Request::~Request()
{
    delete[] _buffer;
    clean();
    if (_socket > 0)
    {
        // pthread_mutex_lock(_host->get_fd_mutex());
        close(_socket);
        // pthread_mutex_unlock(_host->get_fd_mutex());
    }
}

int     Request::read(void)
{
    if (!_end_header)
        return (read_header());
    return (read_body());
}

int     Request::read_header()
{
    // std::cout << "Read header" << std::endl;
    size_t			_header_size;
    int ret = recv(_socket, _buffer, _header_buffer, 0);
    if (ret <= 0)
        return (ret);
    _worker->set_sk_timeout(_socket);
    _buffer[ret] = 0;
    _str_header += _buffer;
    _header_size = _str_header.find("\r\n\r\n");
    if (_header_size == NPOS)
    {
        if (_str_header.size() > _header_buffer)
        {
            pthread_mutex_lock(_host->get_cout_mutex());
            std::cerr << MAGENTA << "Error: No end header found or header too big.\n" << RESET << std::endl;
            pthread_mutex_unlock(_host->get_cout_mutex());
            _status_code = 400;
            return (end_request(1));
        }
        return (ret);
    }
    _end_header = true;
    _body_left = _str_header.size() - _header_size - 4;
    std::string     body_left = _str_header.substr(_header_size + 4);
    if (_body_left > 0)
    {
        std::memmove(_buffer, body_left.c_str(), _body_left);
        _buffer[_body_left] = 0;
    }
    parse_header();
    process_fd_in();
    write_body_left();
    if (_content_length && (_body_size == _content_length))
        return (end_request(3));
    if ((!_chunked && !_content_length) || _method == HEAD)
        return (end_request(4));
    return (ret);
}

void    Request::write_body_left(void)
{
    if (_fd_in == -1 || _body_left <= 0)
        return ;
    if (_chunked)
        write_chunked();
    else
    {
        ssize_t bytes_written = write(_fd_in, _buffer, _body_left);
        if (bytes_written == -1)
        {
            pthread_mutex_lock(_host->get_cout_mutex());
            std::cerr << "Error: Unable to write to file " << _tmp_file << std::endl;
            pthread_mutex_unlock(_host->get_cout_mutex());
            _status_code = 500;
            end_request(5);
        }
        _body_size = _body_left;
    }
    std::memset(_buffer, 0, _body_left);
}

bool	Request::parse_method_url(std::string str)
{
    std::istringstream      iss(str);
    std::string             tk;
    std::string             method;
    int i = 0;
    while (iss >> tk && i < 4)
    {
        if (i == 0)
            method = tk;
        if (i == 1)
            _url = tk;
        i++;
    }
    if (i > 3)
    {
        pthread_mutex_lock(_host->get_cout_mutex());
        std::cerr << RED << "Error: First line header invalid." << RESET << std::endl;
        pthread_mutex_unlock(_host->get_cout_mutex());
        return (false);
    }
    if (method == "GET")
        _method = GET;
    else if (method == "POST")
        _method = POST;
    else if (method == "PUT")
        _method = PUT;
    else if (method == "DELETE")
        _method = DELETE;
	else if (method == "OPTIONS")
		_method = OPTIONS;
    else if (method == "HEAD")
		_method = HEAD;
    else
    {
        pthread_mutex_lock(_host->get_cout_mutex());
        std::cerr << RED << "Error: Method unknown : " << method << RESET << std::endl;
        pthread_mutex_unlock(_host->get_cout_mutex());
        return (false);
    }
    return (true);
}

void	Request::parse_header(void)
{
    size_t          end_line = _str_header.find("\r\n");
    std::string     line = _str_header.substr(0, end_line);
    if (!parse_method_url(line))
        _status_code = 400;
    _str_header.erase(0, end_line + 2);
    size_t          separator;
    do
    {
        end_line = _str_header.find("\r\n");
        if (end_line == NPOS)
            break;
        line = _str_header.substr(0, end_line);
        separator = line.find(": ");
        if (separator != NPOS)
        {
            std::string key = line.substr(0, separator);
            std::string value = line.substr(separator + 2);
            _fields[key] = value;
        }
        _str_header.erase(0, end_line + 2);
    } while (end_line);
    _host_name = _fields["Host"];
    if (_host_name == "")
    {
        pthread_mutex_lock(_host->get_cout_mutex());
        std::cerr << RED << "Error: No host name." << RESET << std::endl;
        pthread_mutex_unlock(_host->get_cout_mutex());
        _status_code = 400;
        // return ;
    }
    std::vector<Server*>*       servers = _address->get_servers();
    std::set<std::string>*      server_names;
    _server = (*servers)[0];
    for (std::vector<Server*>::iterator sv = servers->begin() + 1;
            sv != servers->end(); ++sv)
    {
        server_names = (*sv)->get_server_names();
        if (server_names->find(_host_name) != server_names->end())
            _server = *sv;
    }
	_response.set_server(_server);
    _timeout = _server->get_timeout();
    pthread_mutex_lock(_host->get_cout_mutex());
    ft::timestamp();
    std::cout << GREEN << _location->get_method_str(_method) << " ";
    std::cout << _url << " ";
    std::cout << _host_name << " ";
    std::cout << "[to: " << _timeout << "] ";
    std::cout << "[wk: " << _worker->get_id() << "] ";
    std::cout << "[sk: " << _socket << "]" << RESET << std::endl;
    pthread_mutex_unlock(_host->get_cout_mutex());
    check_location();
    if (_fields["Transfer-Encoding"] == "chunked")
        _chunked = true;
    _content_length = std::atoi(_fields["Content-Length"].c_str());
    _cookies = parse_cookies(_fields["Cookie"]);
    if (_cookies.find("session_id") != _cookies.end())
        _session_id = _cookies["session_id"];
    if (_cookies.find("sid") != _cookies.end())
        _session_id = _cookies["sid"];
    // std::cout << "Session id: " << _session_id << std::endl;
    if (_fields["Connection"] == "close")
        _close = true;
    _content_type = _fields["Content-Type"];
    if (_content_length > _body_max)
        _status_code = 413;
}

std::map<std::string, std::string>    Request::parse_cookies(std::string& str)
{
    if (str == "")
        return (std::map<std::string, std::string>());
    std::map<std::string, std::string>  cookies;
    std::vector<std::string> cookie_pairs = ft::split_string(str, ";");

    for (std::vector<std::string>::iterator it = cookie_pairs.begin();
         it != cookie_pairs.end(); ++it)
    {
        std::string trimmed_cookie = ft::trim_string(*it);
        size_t equal_pos = trimmed_cookie.find('=');
        if (equal_pos != std::string::npos)
        {
            std::string name = trimmed_cookie.substr(0, equal_pos);
            std::string value = trimmed_cookie.substr(equal_pos + 1);
            cookies[name] = value;
        }
    }
    return (cookies);
}

int     Request::read_body()
{
    int     ret;
    ret = recv(_socket, _buffer, _body_buffer, 0);
    if (ret <= 0)
        return (ret);
    _buffer[ret] = 0;
    _worker->set_sk_timeout(_socket);
    if (_chunked)
    {
        write_chunked();
        return (ret);
    }
    _body_size += ret;
    if (_content_length > 1000000)
        ft::print_loading_bar(_body_size, _content_length, 50, _host->get_cout_mutex());
    if (ret > 0 && write(_fd_in, _buffer, ret) == -1)
        return (end_request(ret));
    if (_body_size >= _content_length)
        return (end_request(ret));
    return (ret);
}

static size_t   find_chunk_size(std::string &s, size_t &chunk_size)
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
    size = ft::atoi_base(s.substr(start, end - start).c_str(), "0123456789abcdef");
    if (size == 0 && s.find("\r\n\r\n", start) == NPOS)
        return (NPOS);
    chunk_size = size;
    return (end + 2);
}

void    Request::write_chunked()
{
    size_t          end_size;
    size_t          len;
    size_t          read_size;

    if (!_start_chunked_body)
    {
        _read_data += "\r\n";
        _start_chunked_body = true;
    }
    _read_data += std::string(_buffer);
    read_size = _read_data.size();
    if (!read_size)
    {
        pthread_mutex_lock(_host->get_cout_mutex());
        std::cerr << RED << "Error: No chunked data received." << RESET << std::endl;
        pthread_mutex_unlock(_host->get_cout_mutex());
        return ;
    }
    do
    {
        if (_chunk_size > 0)
        {
            len = _chunk_size;
            if (read_size < len)
                len = read_size;
            if (write(_fd_in, _read_data.c_str(), len) == -1)
            {
                pthread_mutex_lock(_host->get_cout_mutex());
                std::cerr << RED << "Error: Chunked data: Write fd in " << _fd_in << " error." << RESET << std::endl;
                pthread_mutex_unlock(_host->get_cout_mutex());
                _status_code = 500;
                return ;
            }
            _body_size += len;
            ft::print_size(_body_size, _host->get_cout_mutex());
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
            _end = true;
            end_request(9);
            return ;
        }
        _read_data.erase(0, end_size);
        read_size = _read_data.size();
    } while (read_size > 0);
}

bool	Request::check_location()
{
    _location = Location::find_location(_url,
            _server->get_locations(), _method, _status_code);
    if (!_location || _status_code != 200)
        return (false);
    if (_location->get_client_max_body_size() != NPOS)
        _body_max = _location->get_client_max_body_size();
    if (_location->get_redirection())
    {
        _status_code = _location->get_redirection();
        return (true);
    }
    _full_file_name = _location->get_full_file_name(_url,
            _server->get_root(), _method);
    // std::cout << "Full file name: " << _full_file_name << std::endl;
    
    if (_location->get_cgi_pass() != "")
    {
        _cgi = new Cgi(this);
        _cgi->set_pass(_location->get_cgi_pass());
        _cgi->set_file(_full_file_name);
    }
    if (!_cgi && _method != PUT && _method != POST)
    {
        struct stat info;
        if (stat(_full_file_name.c_str(), &info) != 0)
        {
            _status_code = 404;
            return (false);
        }
        if (S_ISDIR(info.st_mode) && _location->get_autoindex())
        {
            _response.set_body(Listing::get_html(&_response));
            _response.set_content_type("text/html");
            return (true);
        }
        // pthread_mutex_lock(_host->get_fd_mutex());
        int     fd_out = open(_full_file_name.c_str(), O_RDONLY);
        // pthread_mutex_unlock(_host->get_fd_mutex());
        if (fd_out == -1)
        {
            _status_code = 500;
	    _host->print(ERROR, "Error: fd out open error: " + ft::itos(fd_out));
            return (false);
        }
        // _host->insert_read_fd(fd_out);
        std::string     extension = ft::file_extension(_full_file_name);
        std::map<std::string, std::string>*     mimes = _host->get_mimes();
        if (mimes->find(extension) != mimes->end())
            _response.set_content_type((*mimes)[extension]);
        else
            _response.set_content_type("plain/text");
        _response.set_fd_out(fd_out);
        _response.set_content_length(info.st_size); 
    }
    return (true);
}

void	Request::process_fd_in()
{
    std::string     tmp_file_prefix = "/tmp/" + ft::itos(_worker->get_id()) + "_";
    int i = 0;
    switch (_method)
    {
		case OPTIONS:
        case HEAD:
        case GET:
            break;
        case PUT:
            if (_status_code == 200)
            {
                // pthread_mutex_lock(_host->get_fd_mutex());
                _fd_in = open(_full_file_name.c_str(),
                        O_CREAT | O_WRONLY | O_TRUNC, 0664);
                // pthread_mutex_unlock(_host->get_fd_mutex());
                if (_fd_in == -1)
                {
                    _host->print(ERROR, "Error: fd in open error: " + _full_file_name);
                    // pthread_mutex_lock(_host->get_cout_mutex());
                    // std::cerr << RED << "Error: Can not open file " << _full_file_name << "." << RESET << std::endl;
                    // pthread_mutex_unlock(_host->get_cout_mutex());
                    _status_code = 403;
                }
            }
            break;
        case POST:
            _tmp_file = tmp_file_prefix + "0";
            struct stat buffer;
            // pthread_mutex_lock(_host->get_fd_mutex());
            while (stat(_tmp_file.c_str(), &buffer) == 0)
                _tmp_file = tmp_file_prefix + ft::itos(++i);
            _fd_in = open(_tmp_file.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0664);
            // pthread_mutex_unlock(_host->get_fd_mutex());
            if (_fd_in == -1)
            {
                _host->print(ERROR, "Error: fd in open error: " + _tmp_file);
                _status_code = 500;
                end_request(7);
            }
            break;
        case DELETE:
            if (_status_code == 200 && std::remove(_full_file_name.c_str()))
            {
                _host->print(ERROR, "Error: Can not delete file: " + _full_file_name);
                _status_code = 500;
                end_request(8);
            }
            break;
        case NONE:
            break;
    }
}

int     Request::end_request(int ret)
{
    if (_status_code == 200 && _cgi)
        _status_code = _cgi->execute();
    if (_status_code == 200 && _body_size > _body_max)
        _status_code = 413;
    if (_fd_in > 0)
    {
        // pthread_mutex_lock(_host->get_fd_mutex());
        close(_fd_in);
        // pthread_mutex_unlock(_host->get_fd_mutex());
        _fd_in = -1;
    }
    _end = true;
    _response.set_status_code(_status_code);
    if (*(_response.get_header()) == "")
        _response.header_generate();
    // _response.header_generate();
    return (ret);
}

Host*               Request::get_host(void) const {return (_host);}
Server*	            Request::get_server(void) const {return (_server);}
e_method            Request::get_method(void) const {return (_method);}
std::string         Request::get_url(void) const {return (_url);}
Response*           Request::get_response(void) {return (&_response);}
Cgi*                Request::get_cgi(void) const {return (_cgi);}
int                 Request::get_status_code(void) const {return (_status_code);}
std::string	    Request::get_content_type(void) const {return (_content_type);}
size_t		    Request::get_content_length(void) const {return (_content_length);}
size_t		    Request::get_body_size(void) const {return (_body_size);}
std::string*	Request::get_str_header(void) {return (&_str_header);}
std::string	    Request::get_full_file_name(void) const {return (_full_file_name);}
Location*	    Request::get_location(void) const {return (_location);}
int		        Request::get_fd_in(void) const {return (_fd_in);}
std::string	    Request::get_session_id(void) const {return (_session_id);}
bool		    Request::get_end(void) const {return (_end);}
bool		    Request::get_chunked(void) const {return (_chunked);}
int		        Request::get_timeout(void) const {return (_timeout);}
std::map<std::string, std::string>*     Request::get_fields(void) {return (&_fields);}
bool		    Request::get_close(void) const {return (_close);}
Worker*			Request::get_worker(void) const {return (_worker);}
int				Request::get_socket(void) const {return (_socket);}

void		    Request::set_fd_in(int f) {_fd_in = f;}
void            Request::set_cgi(Cgi* c) {_cgi = c;}
