/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nbechon <nbechon@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/02/21 20:49:43 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/types.h>
#include <sys/wait.h>


#include <cstring>
#include <unistd.h>

#include "Host.hpp"
#include "Server.hpp"
#include "Location.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "webserv.hpp"

#include "Cgi.hpp"

#define BUFFER_SIZE 65792

Cgi::Cgi() {}
Cgi::Cgi(Request* request): _request(request)
{
    _envs = 0;
    _pid = -1;
    _tmp_file = "";
    _content_length = 0;
}
Cgi::Cgi(const Cgi& src) { *this = src; }
Cgi&	Cgi::operator=( Cgi const & src )
{
	(void) src;
	return (*this);
}
Cgi::~Cgi()
{
    if (_envs)
    {
        int i = 0;
        while (_envs[i])
            free(_envs[i++]);
        delete[] _envs;
    }
    if (_fd_out != -1)
    {
        _request->get_response()->set_fd_out(-1);
        close(_fd_out);
    }
    if (_tmp_file != "" && std::remove(_tmp_file.c_str()))
        std::cerr << MAGENTA << "Error: Can not delete file " << _tmp_file << RESET << std::endl;
}


int    Cgi::execute()
{
    Response*       response;

    response = _request->get_response();
    if (!get_envs())
    {
        std::cerr << "Error: envs" << std::endl;
        response->set_status_code(500);
        return 500;
    }

    std::cout << "Cgi execute" << std::endl;
    
    int pip[2];
    
    if (pipe(pip) == -1) {
        std::cerr << "Error: pipe" << std::endl;
        perror("pipe");
        return 500;
    }

    _tmp_file = "/tmp/1";
    struct stat buffer;
    int i = 0;
    while (stat(_tmp_file.c_str(), &buffer) == 0)
        _tmp_file = "/tmp/" + ft::itos(++i);
    _fd_out = open(_tmp_file.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0664);
    if (_fd_out == -1)
    {
        std::cerr << "Error: CGI fd_out open error." << std::endl;
        return 500;
    }
    std::cerr << "Cgi _fd_out:" << _fd_out << std::endl;
    response->set_fd_out(_fd_out);

    _pid = fork();

    if (_pid == -1)
    {
        std::cerr << "Error: fork" << std::endl;
        return 500;
    }
    else if (!_pid)
    {
        close(pip[1]);
        if (dup2(pip[0], STDIN_FILENO) == -1)
            return (-1);
        if (dup2(_fd_out, STDOUT_FILENO) == -1)
            return 500;
        close(pip[0]);
        //std::cout << _pass.c_str() << std::endl;
        //std::cout << _file.c_str() << std::endl;
        
        char*   argv[3];
        argv[0] = (char*) _pass.c_str();
        argv[1] = (char*) _file.c_str();
        argv[2] = 0;
        //execve(argv[0], argv, _envs);
        execve(argv[0], argv, _envs);
        //execlp("cat", "cat", nullptr);
        
        std::cerr << "Execution error" << std::endl;
        return (500);
    }
    else
    {
        char    buffer[BUFFER_SIZE + 1];
        ssize_t bytesRead;

        close(pip[0]);
        int     fd_in = _request->get_fd_in();
        if (fd_in != -1)
        {
            if (lseek(fd_in, 0, SEEK_SET) == -1) {
                std::cerr << "Error: using lseek" << std::endl;
                return 500;
            }
            std::cout << "fd_in: " << fd_in << std::endl;
            while ((bytesRead = read(fd_in, buffer, BUFFER_SIZE)) > 0)
            {
                buffer[bytesRead] = 0;
                //std::cout << buffer;
                if (write(pip[1], buffer, bytesRead) == -1)
                {
                    std::cerr << "Error: write pipe in" << std::endl;
                    return 500;
                }
            }
            //std::cout << std::endl;
            close(fd_in);
        }
        close(pip[1]);
        int     status;
        if (waitpid(_pid, &status, 0) == -1)
            return 500;
        if (WIFEXITED(status) && WEXITSTATUS(status))
            return 502;
        std::cout << "Cgi output: " << status << std::endl;
        return parse_header();
    }
}

int    Cgi::parse_header()
{
    std::string     header = "";
    char            buffer[BUFFER_SIZE + 1];
    ssize_t         ret;
    size_t          pos;
    bool            header_complete = false;

    if (lseek(_fd_out, 0, SEEK_SET) == -1)
    {
        std::cerr << "Error: cgi _fd_out using lseek" << std::endl;
        return 500;
    }
    //std::cout << "Cgi header:'" << std::endl;
    while ((ret = read(_fd_out, buffer, BUFFER_SIZE)) > 0 && !header_complete) {
        buffer[ret] = '\0';
        //std::cout << buffer;
        header += buffer;
        pos = header.find("\r\n\r\n");
        if (pos != std::string::npos) {
            header_complete = true;
            header = header.substr(0, pos + 4);
        }
    }
    //std::cout << "`" << std::endl;
    
    size_t status_pos = header.find("Status: ");
    if (status_pos != std::string::npos) {
        size_t code_start = status_pos + strlen("Status: ");
        size_t code_end = header.find(" ", code_start);
        if (code_end != std::string::npos) {
            std::string code_str = header.substr(code_start, code_end - code_start);
            _status_code = atoi(code_str.c_str());
            size_t message_start = code_end + 1;
            size_t message_end = header.find("\n", message_start);
            if (message_end != std::string::npos) {
                _status_message = header.substr(message_start, message_end - message_start);
            }
        }
    }
    size_t content_type_pos = header.find("Content-Type: ");
    if (content_type_pos != std::string::npos) {
        size_t value_start = content_type_pos + strlen("Content-Type: ");
        size_t space_pos = header.find(" ", value_start);
        if (space_pos != std::string::npos) {
            _content_type = header.substr(value_start, space_pos - value_start);
        } else {
            _content_type = header.substr(value_start);
        }
    }
    pos = header.find("\r\n\r\n");
    if (header == "" || pos == NPOS)
    {
        std::cerr << RED << "Error: No header found in cgi _fd_out" << RESET << std::endl;
        return 500;
    }
    off_t body_start = pos + 4;
    if (lseek(_fd_out, body_start, SEEK_SET) == -1) {
        std::cerr << "Error: failed to position _fd_out at the beginning of the body" << std::endl;
        return 500;
    }
    struct stat fileStat;
    if (fstat(_fd_out, &fileStat) == -1)
    {
        std::cerr << RED << "Error: Cgi getting file information" << std::endl;
        return 500;
        
    }
    _content_length = fileStat.st_size - body_start;
    std::cout << "Cgi content_length: " << _content_length << std::endl;
    // ret = read(_fd_out, buffer, 4);
    // buffer[ret] = '\0';
    // std::cout << "`" << buffer << "`" << std::endl;
    return (_status_code);
}

bool    Cgi::get_envs()
{
    //std::cout << "Cgi get_envs" << std::endl;
    std::vector<std::string>  envs;

    if (_request->get_method() == POST) {
        envs.push_back("CONTENT_TYPE=" + _request->get_content_type());
        envs.push_back("CONTENT_LENGTH=" + ft::itos((int) _request->get_content_length()));
    }
    envs.push_back("GATEWAY_INTERFACE=CGI/1.1");
    envs.push_back("PATH_INFO=" + _file);
    envs.push_back("PATH_TRANSLATED=" + _file);
    envs.push_back("QUERY_STRING=");
    envs.push_back("REMOTE_ADDR=...");
    envs.push_back("REQUEST_METHOD=" + Location::get_method_str(_request->get_method()));
    envs.push_back("REQUEST_URI=" + _file);

    Server*   server = _request->get_server();
    envs.push_back("SCRIPT_NAME=" + _pass);
    envs.push_back("SERVER_NAME=" + std::string(server->get_ip_address()));
    envs.push_back("SERVER_PORT=" + ft::itos((int) server->get_port()));
    envs.push_back("SERVER_PROTOCOL=HTTP/1.1");
    envs.push_back("SERVER_SOFTWARE=WEBSERV/1.0");

    std::string extension = ft::file_extension(_request->get_full_file_name());
    if (extension == "php")
        envs.push_back("REDIRECT_STATUS=200");

    
    if (!(_envs = new char*[envs.size() + 1]))
        return false;

    size_t          i;
    i = 0;
    for (std::vector<std::string>::iterator it = envs.begin();
            it != envs.end(); it++)
    {
        //std::cout << *it << std::endl;
        _envs[i++] = strdup(it->c_str());
    }
    _envs[i] = 0;
    return (true);
}

int             Cgi::get_pid(void) const {return (_pid);}
int             Cgi::get_status_code(void) const {return (_status_code);}
std::string     Cgi::get_status_message(void) const {return (_status_message);}
std::string     Cgi::get_content_type(void) const {return (_content_type);}
size_t          Cgi::get_content_length(void) const {return (_content_length);}

void        Cgi::set_request(Request* r) {_request = r;}
void        Cgi::set_file(std::string f) {_file = f;}
void        Cgi::set_pass(std::string p) {_pass = p;}
