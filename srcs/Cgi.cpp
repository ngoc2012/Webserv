/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nbechon <nbechon@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/03/04 21:03:57 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>
#include <unistd.h>
#include <cerrno> // For errno
#include <cstring> // For strerror

#include "Host.hpp"
#include "Address.hpp"
#include "Server.hpp"
#include "Location.hpp"
#include "Request.hpp"
#include "Worker.hpp"
#include "Response.hpp"
#include "webserv.hpp"

#include "Cgi.hpp"

#define BUFFER_SIZE 65792

Cgi::Cgi() {}
Cgi::Cgi(Request* request): _request(request)
{
    _fd_out = -1;
    _envs = 0;
    _pid = -1;
    _tmp_file = "";
    _content_length = 0;
    _pipe[0] = -1;
    _pipe[1] = -1;
    _response = _request->get_response();
	_pipe_close = false;
    pthread_cond_init(&_cond_exec_queue, NULL);
}
Cgi::Cgi(const Cgi& src) { *this = src; }
Cgi&	Cgi::operator=( Cgi const & src )
{
	(void) src;
	return (*this);
}
Cgi::~Cgi()
{
    pthread_mutex_lock(_request->get_host()->get_cout_mutex());
    std::cerr << "Cgi destructed, wk = " << _request->get_worker()->get_id() << ", sk = " << _request->get_socket() << std::endl;
    pthread_mutex_unlock(_request->get_host()->get_cout_mutex());
    if (_pid != -1)
    {
        pthread_mutex_lock(_request->get_host()->get_cout_mutex());
        std::cerr << "Cgi: Kill on destruction, wk = " << _request->get_worker()->get_id() << ", sk = " << _request->get_socket() << ", pid=" << _pid << std::endl;
        pthread_mutex_unlock(_request->get_host()->get_cout_mutex());
        kill(_pid, SIGTERM);
    }
    pthread_mutex_t*	fd_mutex = _request->get_host()->get_fd_mutex();
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
        pthread_mutex_lock(fd_mutex);
        close(_fd_out);
        _fd_out = -1;
        pthread_mutex_unlock(fd_mutex);
    }
    if (_tmp_file != "" && std::remove(_tmp_file.c_str()))
    {
        pthread_mutex_lock(_request->get_host()->get_cout_mutex());
        std::cerr << MAGENTA << "Error: Can not delete file " << _tmp_file << RESET << std::endl;
        pthread_mutex_unlock(_request->get_host()->get_cout_mutex());
    }
    pthread_cond_destroy(&_cond_exec_queue);
}

static void signalHandler(int signum) {
    // std::cerr << "Child process received signal: " << signum << std::endl;
    exit(signum);
}

int    Cgi::execute()
{
    pthread_mutex_t*	fd_mutex = _request->get_host()->get_fd_mutex();
    pthread_mutex_t*	cout_mutex = _request->get_host()->get_cout_mutex();

    if (!get_envs())
    {
        pthread_mutex_lock(cout_mutex);
        std::cerr << "Error: envs" << std::endl;
        pthread_mutex_unlock(cout_mutex);
        return 500;
    }
    std::string     tmp_file_prefix = "/tmp/cgi_" + ft::itos(_request->get_worker()->get_id()) + "_";
    _tmp_file = tmp_file_prefix + "0";
    struct stat buffer;
    int i = 0;
	
    pthread_mutex_lock(fd_mutex);
    if (pipe(_pipe) == -1) {
        pthread_mutex_lock(cout_mutex);
        std::cerr << "Error: pipe" << std::endl;
        pthread_mutex_unlock(cout_mutex);
        return 500;
    }
    // pthread_mutex_lock(cout_mutex);
    // std::cout << "Cgi: create pipe: " << _pipe[0] << "|" << _pipe[1] << "|" << _request->get_worker()->get_id()  << std::endl;
    // pthread_mutex_unlock(cout_mutex);
    while (stat(_tmp_file.c_str(), &buffer) == 0)
        _tmp_file = tmp_file_prefix + ft::itos(++i);
    _fd_out = open(_tmp_file.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0664);
	pthread_mutex_unlock(fd_mutex);
    // pthread_mutex_lock(cout_mutex);
    // std::cout << "Cgi: open fd out: " << _fd_out << "|" << _request->get_worker()->get_id()  << std::endl;
    // pthread_mutex_unlock(cout_mutex);

    if (_fd_out == -1)
    {
        std::cerr << "Error: CGI fd_out open error." << std::endl;
        return 500;
    }
    _response->set_fd_out(_fd_out);
    // std::cout << _request->get_host() << std::endl;
    _pid = fork();

    if (_pid == -1)
    {
        std::cerr << "Error: fork" << std::endl;
        return 500;
    }
    else if (!_pid)
    {
        // Host*   host = _request->get_host();
        // // Cgi*   cgi = this;
        // if (_fd_out > 0)
        //     close(_fd_out);
        // close(_pipe[0]);
        // close(_pipe[1]);
        // int     fd_in = _request->get_fd_in();
        // if (fd_in != -1)
        //     close(fd_in);
        // std::map<int, Address*>*     _sk_address = host->get_sk_address();
        // for (std::map<int, Address*>::iterator it = _sk_address->begin();
        // it != _sk_address->end(); it++)
        // {
        //     if (it->second->get_listen_socket() > 0)
        //         close(it->second->get_listen_socket());
        //     delete it->second;
        //     // std::vector<Server*>*   _servers = it->second->get_servers();
        //     // for (std::vector<Server*>::iterator it = _servers->begin();
        //     // it != _servers->end(); ++it)
        //     //     delete (*it);
        //     // _servers->clear();
        // }
        // host->get_mimes()->clear();
        // host->get_set_mimes()->clear();
        // host->get_status_message()->clear();
        // std::map<int, Worker*>*     _sk_worker = host->get_sk_worker();
        // for (std::map<int, Worker*>::iterator it = _sk_worker->begin();
        // it != _sk_worker->end(); it++)
        //     close(it->first);
        // Worker*     workers = host->get_workers();
        // for (int i = 0; i < host->get_n_workers(); i++)
        //     delete &workers[i];
        // for (std::map<int, Request*>::iterator it = _sk_request->begin();
        //     it != _sk_request.end(); ++it)
        // delete (it->second);
        
        // for (int i = 0; i < host->get_n_workers(); i++)
        //     host->get_workers()[i].set_end();
        // for (int i = 0; i < host->get_n_workers(); i++)
        //     pthread_join(*(host->get_workers()[i].get_th()), NULL);
        // std::cout << host << std::endl;
        // int     fd_out = _fd_out;
        // int     pipe[2];
        // pipe[0] = _pipe[0];
        // pipe[1] = _pipe[1];
        // close(pipe[1]);
        // if (dup2(pipe[0], STDIN_FILENO) == -1)
        //     return 500;
        // if (dup2(fd_out, STDOUT_FILENO) == -1)
        //     return 500;
        // close(pipe[0]);
        
        // char*   argv[3];
        // argv[0] = ft::strdup(_pass.c_str());
        // argv[1] = ft::strdup(_file.c_str());
        // argv[2] = 0;
        // _request->set_cgi(0);
        // host->set_end(true);
        // delete host;
        // std::cout << _envs << std::endl;
        // (void) host;
        // (void) cgi;
        // execve(argv[0], argv, _envs);
        // std::cerr << "Execution error" << std::endl;
        // delete argv[0];
        // delete argv[1];
        // delete cgi;
        // return (500);
        // exit(500);

        signal(SIGTERM, signalHandler);

        pthread_mutex_lock(cout_mutex);
        std::cout << "Cgi: fork start child process: wk = " << _request->get_worker()->get_id() << ", sk = " << _request->get_socket() << ", pid=" << getpid() << std::endl;
        pthread_mutex_unlock(cout_mutex);
		// ===================================================================================================
        pthread_mutex_lock(fd_mutex);
        close(_pipe[1]);
        // pthread_mutex_lock(cout_mutex);
        // std::cout << "Cgi: close pipe1_child: " << _pipe[1] << "|" << _request->get_worker()->get_id()  << std::endl;
        // pthread_mutex_unlock(cout_mutex);
        if (dup2(_pipe[0], STDIN_FILENO) == -1)
        {
            pthread_mutex_unlock(fd_mutex);
            pthread_mutex_lock(cout_mutex);
            std::cerr << "Error: dup2" << std::endl;
            pthread_mutex_unlock(cout_mutex);
            return 500;
        }
        // pthread_mutex_lock(cout_mutex);
        // std::cout << "Cgi: child: dup2: " << _pipe[0] << " to stdin|" << _request->get_worker()->get_id()  << std::endl;
        // pthread_mutex_unlock(cout_mutex);
        if (dup2(_fd_out, STDOUT_FILENO) == -1)
        {
            pthread_mutex_unlock(fd_mutex);
            pthread_mutex_lock(cout_mutex);
            std::cerr << "Error: dup2" << std::endl;
            pthread_mutex_unlock(cout_mutex);
            return 500;
        }
        // pthread_mutex_lock(cout_mutex);
        // std::cout << "Cgi: child: dup2: " << _fd_out << "to stdout|" << _request->get_worker()->get_id()  << std::endl;
        // pthread_mutex_unlock(cout_mutex);
        close(_pipe[0]);
        pthread_mutex_unlock(fd_mutex);
		// ===================================================================================================
        // pthread_mutex_lock(cout_mutex);
        // std::cout << "Cgi: close pipe0_child: " << _pipe[0] << "|" << _request->get_worker()->get_id()  << std::endl;
        // pthread_mutex_unlock(cout_mutex);
        
        char*   argv[3];
        argv[0] = (char*) _pass.c_str();
        argv[1] = (char*) _file.c_str();
        argv[2] = 0;
        // pthread_mutex_lock(cout_mutex);
        // std::cout << "execute cgi: " << _request->get_worker()->get_id()  << std::endl;
        // pthread_mutex_unlock(cout_mutex);
        execve(argv[0], argv, _envs);
        std::cerr << "Execution error" << std::endl;
        return (500);
    }
    else
    {
        char    buffer[BUFFER_SIZE + 1];
        ssize_t bytesRead;

        int     fd_in = _request->get_fd_in();
        if (fd_in != -1)
        {
            if (lseek(fd_in, 0, SEEK_SET) == -1) {
                std::cerr << "Error: using lseek" << std::endl;
                return 500;
            }
            while ((bytesRead = read(fd_in, buffer, BUFFER_SIZE)) > 0)
            {
                buffer[bytesRead] = 0;
                if (write(_pipe[1], buffer, bytesRead) == -1)
                {
                    // pthread_mutex_lock(cout_mutex);
                    // std::cerr << "Error: Cgi write pipe in: " << _pipe[1] << "|" << strerror(errno) << std::endl;
                    // pthread_mutex_unlock(cout_mutex);
                    return 500;
                }
            }
            if (bytesRead == -1)
            {
                // pthread_mutex_lock(cout_mutex);
                // std::cerr << "Error: Cgi read fd_in: " << fd_in << "|" << strerror(errno) << std::endl;
                // pthread_mutex_unlock(cout_mutex);
                return 500;
            }
            // pthread_mutex_lock(cout_mutex);
            // std::cout << "Cgi: close fd in: " << fd_in << "|" << _request->get_worker()->get_id()  << std::endl;
            // pthread_mutex_unlock(cout_mutex);
            pthread_mutex_lock(fd_mutex);
            close(fd_in);
            fd_in = -1;
            _request->set_fd_in(-1);
            pthread_mutex_unlock(fd_mutex);
        }
        pthread_mutex_lock(fd_mutex);
        close(_pipe[1]);
        close(_pipe[0]);
        pthread_mutex_unlock(fd_mutex);

        /*
        int     status;
        if (waitpid(_pid, &status, 0) == -1)
            return 500;
        if (WIFEXITED(status) && WEXITSTATUS(status))
            return 502;
        pthread_mutex_lock(cout_mutex);
        std::cout << "End execute cgi: " << _request->get_worker()->get_id()  << std::endl;
        pthread_mutex_unlock(cout_mutex);
        */
        
        int     timeout = _request->get_timeout() - 1;
        if (timeout < 1)
            timeout = 1;

        time_t start_time = time(0);
        int status;
        while (waitpid(_pid, &status, WNOHANG) == 0) {
            time_t current_time = time(0);
            if (current_time - start_time > timeout) {
                kill(_pid, SIGTERM);
                pthread_mutex_lock(cout_mutex);
                std::cout << "Cgi: Kill child process: wk = " << _request->get_worker()->get_id() << ", sk = " << _request->get_socket() << ", pid=" << _pid << std::endl;
                pthread_mutex_unlock(cout_mutex);
                waitpid(_pid, &status, 0);
                _pid = -1;
                return 504;
            }
        }
        std::cout << "Cgi: Child process return: wk = " << _request->get_worker()->get_id() << ", sk = " << _request->get_socket() << ", pid=" << _pid << std::endl;
        _pid = -1;
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
    while ((ret = read(_fd_out, buffer, BUFFER_SIZE)) > 0 && !header_complete) {
        buffer[ret] = '\0';
        header += buffer;
        pos = header.find("\r\n\r\n");
        if (pos != std::string::npos)
        {
            header_complete = true;
            header = header.substr(0, pos + 4);
        }
    }
    
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
    _response->set_content_length(_content_length);
    _response->set_content_type(_content_type);
    return (_status_code);
}

bool    Cgi::get_envs()
{
    std::vector<std::string>  envs;
    if (_request->get_method() == POST) {
        envs.push_back("CONTENT_TYPE=" + _request->get_content_type());
        envs.push_back("CONTENT_LENGTH=" + ft::itos((int) _request->get_content_length()));
    }
    envs.push_back("GATEWAY_INTERFACE=CGI/1.1");
    std::map<std::string, std::string>*     fields = _request->get_fields();
	for (std::map<std::string, std::string>::iterator f = fields->begin();
		f != fields->end(); f++)
    {
        if (f->first != "Host" && f->first != "Content-Type" && f->first != "Content-Length")
            envs.push_back("HTTP_" + ft::to_upper(f->first) + "=" + f->second);

    }
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
        _envs[i++] = strdup(it->c_str());
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
