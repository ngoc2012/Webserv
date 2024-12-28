/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Host.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/03/06 20:09:03 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ctime>

#include "Host.hpp"
#include "Worker.hpp"
#include "Address.hpp"
#include "Server.hpp"
#include "Response.hpp"
#include "Request.hpp"
#include "Configuration.hpp"

Host::Host(const Host& src) { *this = src; }

Host::Host()
{
    _end = false;
    _stop = 0;
    _max_clients = 128;
    _client_max_body_size = 1024;
    _client_body_buffer_size = 128;
    _large_client_header_buffer = 8;
    _parser_error = false;
    _workers = 0;
    _start_worker_id = 0;
    _n_workers = 1;
    _max_sk = -1;
    pthread_mutex_init(&_cout_mutex, NULL);
    pthread_mutex_init(&_set_mutex, NULL);
    pthread_mutex_init(&_stop_mutex, NULL);
    pthread_mutex_init(&_fd_mutex, NULL);
    pthread_mutex_init(&_sk_worker_mutex, NULL);
    _timeout = TIMEOUT;
    mimes();
    status_message();
}

Host&	Host::operator=( Host const & src )
{
	(void) src;
	return (*this);
}

Host::~Host()
{
    for (std::map<std::string, Address*>::iterator it = _str_address.begin();
            it != _str_address.end(); ++it)
        delete (it->second);
    if (_workers)
        delete [] _workers;
    pthread_mutex_destroy(&_cout_mutex);
    pthread_mutex_destroy(&_set_mutex);
    pthread_mutex_destroy(&_stop_mutex);
    pthread_mutex_destroy(&_fd_mutex);
    pthread_mutex_destroy(&_sk_worker_mutex);
    // Cout mutex destroyed in main
    ft::timestamp();
    std::cout << "End server" << std::endl;
}

void	Host::start(void)
{
    
    if (_parser_error)
    	return ;
    FD_ZERO(&_listen_set);
    FD_ZERO(&_master_read_set);
    FD_ZERO(&_master_write_set);
    start_server();
    if (!start_workers())
    	return ;
    if (!_sk_address.size())
    	return ;
    do
    {
        pthread_mutex_lock(&_set_mutex);
        memcpy(&_read_set, &_master_read_set, sizeof(_master_read_set));
        memcpy(&_write_set, &_master_write_set, sizeof(_master_write_set));
        struct timeval timeout;
        timeout.tv_sec = 3;
        timeout.tv_usec = 0;
        if (select(_max_sk + 1, &_read_set, &_write_set, NULL, &timeout) != -1)
        {
            pthread_mutex_unlock(&_set_mutex);
            check_sk_ready();
        }
        else
            pthread_mutex_unlock(&_set_mutex);
        usleep(DELAY);
    } while (!get_stop());
}

void    Host::round_robin(int new_sk, Address* address)
{
    int i = 0;
    int w_min = (i + _start_worker_id) % _n_workers;
    int j = (i + _start_worker_id + 1) % _n_workers;
    while (i < _n_workers - 1)
    {
        //if (_worker_load[&_workers[w_min]] >= _worker_load[&_workers[j]])
        if (_workers[w_min].get_workload() >= _workers[j].get_workload())
            w_min = j;
        j = (j + 1) % _n_workers;
        i++;
    }
    pthread_mutex_lock(&_sk_worker_mutex);
    _sk_worker[new_sk] = &_workers[w_min];
    _workers[w_min].new_connection(new_sk, address);
    pthread_mutex_unlock(&_sk_worker_mutex);
    //_worker_load[&_workers[w_min]]++;
    _workers[w_min].set_workload(1);
    _start_worker_id++;
    _start_worker_id %= _n_workers;
}

void	Host::check_sk_ready(void)
{
    int     	new_sk;
    int		listen_sk;
    Address*	listen_addr;

    for (std::map<int, Address*>::iterator it = _sk_address.begin();
        it != _sk_address.end(); it++)
    {
    	listen_sk = it->first;
    	listen_addr = it->second;
        if (FD_ISSET(listen_sk, &_read_set))
        {
            new_sk = listen_addr->accept_client_sk();
            if (new_sk < 0)
                continue;
            pthread_mutex_lock(&_set_mutex);
            if (new_sk > _max_sk)
                _max_sk = new_sk;
            FD_SET(new_sk, &_master_read_set);
            FD_SET(new_sk, &_master_write_set);
            pthread_mutex_unlock(&_set_mutex);
            round_robin(new_sk, listen_addr);
            usleep(DELAY);
        }
    }

    pthread_mutex_lock(&_sk_worker_mutex);
    std::map<int, Worker*>		sk_worker = _sk_worker;
    pthread_mutex_unlock(&_sk_worker_mutex);

    for (int i = 0; i < _n_workers; i++)
    	if (_workers[i].get_workload())
        {
            _workers[i].set_set_updated(true);
            pthread_mutex_lock(_workers[i].get_set_mutex());
            pthread_cond_signal(_workers[i].get_cond_set_updated());
            pthread_mutex_unlock(_workers[i].get_set_mutex());
        }
}

void  	Host::insert_read_fd(int fd)
{
    pthread_mutex_lock(&_set_mutex);
    // print(NORMAL, "read fd insert: " + ft::itos(fd));
    FD_SET(fd, &_master_read_set);
    pthread_mutex_unlock(&_set_mutex);
}

void  	Host::clear_read_fd(int fd)
{
    pthread_mutex_lock(&_set_mutex);
    // print(NORMAL, "read fd clear: " + ft::itos(fd));
    FD_CLR(fd, &_master_read_set);
    pthread_mutex_unlock(&_set_mutex);
}

void  	Host::close_connection(int i)
{
    print(SUCCESS, "Close connection " + ft::itos(i));
    pthread_mutex_lock(&_set_mutex);
    FD_CLR(i, &_master_read_set);
    FD_CLR(i, &_master_write_set);
    if (i == _max_sk)
        while (!FD_ISSET(_max_sk, &_master_read_set))
            _max_sk -= 1;
    pthread_mutex_unlock(&_set_mutex);
    pthread_mutex_lock(&_sk_worker_mutex);
    _sk_worker[i]->set_workload(-1);
    _sk_worker.erase(i);
    pthread_mutex_unlock(&_sk_worker_mutex);
}

void	Host::start_server(void)
{
    int	listen_sk;
    for (std::map<std::string, Address*>::iterator ad = _str_address.begin();
    ad != _str_address.end();)
    {
        listen_sk = (ad->second)->listen_socket();
        if (listen_sk > 0)
        {
            if (listen_sk > _max_sk)
                _max_sk = listen_sk;
            FD_SET(listen_sk, &_listen_set);
            insert_read_fd(listen_sk);
            _sk_address[listen_sk] = ad->second;
            ++ad;
        } else {
            delete (ad->second);
            _str_address.erase(ad++);
        }
    }
}

static void*   start_worker(void* instance)
{
    if (!instance)
        return NULL;
    Worker* worker = static_cast<Worker*>(instance);
    Host*   host = worker->get_host();

    host->print(SUCCESS, "Worker " + ft::itos(worker->get_id()) + " started.");

    while (!worker->get_terminate_flag())
    {
        worker->check_timeout();
        if (!worker->get_workload())
        {
            worker->wait_for_set_updated();
            if (worker->get_terminate_flag())
                break;
            worker->routine();
        }
        else
        {
            if (worker->get_set_updated())
            {
                worker->set_set_updated(false);
                worker->routine();
            }
            else
                usleep(DELAY);
        }
    }
    host->print(SUCCESS, "Worker " + ft::itos(worker->get_id()) + " ended.");
    pthread_exit(NULL);
    return NULL;
}

void  	Host::print(e_message m, std::string str)
{
    pthread_mutex_lock(&_cout_mutex);
    ft::timestamp();
    if (m == ERROR)
        std::cerr << RED << str << RESET << std::endl;
    else if (m == WARNING)
        std::cerr << YELLOW << str << RESET << std::endl;
    else if (m == SUCCESS)
        std::cout << GREEN << str << RESET << std::endl;
    else
        std::cout << str << std::endl;
    pthread_mutex_unlock(&_cout_mutex);
}

bool    Host::start_workers() {
    _workers = new Worker[_n_workers];
    for (int i = 0; i < _n_workers; i++)
    {
        _workers[i].set_id(i);
        _workers[i].set_host(this);
        if (pthread_create(_workers[i].get_th(), NULL, start_worker, &_workers[i]))
        {
            print(ERROR, "Error creating worker thread");
            return (false);
        }
    }
    return (true);
}

bool	Host::is_writable_fd(int fd)
{
    if (fd < 0)
	return false;
    pthread_mutex_lock(&_set_mutex);
    bool	is_writable = FD_ISSET(fd, &_write_set);
    pthread_mutex_unlock(&_set_mutex);
    return is_writable;
}

bool	Host::is_readable_fd(int fd)
{
    if (fd < 0)
	return false;
    pthread_mutex_lock(&_set_mutex);
    bool	is_readable = FD_ISSET(fd, &_read_set);
    pthread_mutex_unlock(&_set_mutex);
    return is_readable;
}

void    Host::status_message(void)
{
    _status_message[100] = "Continue";
    _status_message[200] = "OK";
    _status_message[201] = "Created";
    _status_message[204] = "No Content";
    _status_message[301] = "Moved Permanently";
    _status_message[302] = "Found";
    _status_message[303] = "See Other";
    _status_message[307] = "Temporary Redirect";
    _status_message[308] = "Permanent Redirect";
    _status_message[400] = "Bad Request";
    _status_message[403] = "Forbidden";
    _status_message[404] = "Not Found";
    _status_message[405] = "Method Not Allowed";
    _status_message[413] = "Payload Too Large";
    _status_message[499] = "Client Closed Request";
    _status_message[500] = "Internal Server Error";
    _status_message[502] = "Bad Gateway";
    _status_message[504] = "Gateway Timeout";
}

void	Host::mimes(void)
{
    _mimes["html"] = "text/html";
    _mimes["htm"] = "text/html";
    _mimes["shtml"] = "text/html";
    _mimes["css"] = "text/css";
    _mimes["xml"] = "text/xml";
    _mimes["gif"] = "image/gif";
    _mimes["jpeg"] = "image/jpeg";
    _mimes["jpg"] = "image/jpeg";
    _mimes["js"] = "application/javascript";
    _mimes["es"] = "application/javascript";
    _mimes["atom"] = "application/atom+xml";
    _mimes["rss"] = "application/rss+xml";

    _mimes["mml"] = "text/mathml";
    _mimes["txt"] = "text/plain";
    _mimes["jad"] = "text/vnd.sun.j2me.app-descriptor";
    _mimes["wml"] = "text/vnd.wap.wml";
    _mimes["htc"] = "text/x-component";

    _mimes["png"] = "image/png";
    _mimes["svg"] = "image/svg+xml";
    _mimes["svgz"] = "image/svg+xml";
    _mimes["tif"] = "image/tiff";
    _mimes["tiff"] = "image/tiff";
    _mimes["wbmp"] = "image/vnd.wap.wbmp";
    _mimes["webp"] = "image/webp";
    _mimes["ico"] = "image/x-icon";
    _mimes["jng"] = "image/x-jng";
    _mimes["bmp"] = "image/x-ms-bmp";

    _mimes["woff"] = "font/woff";
    _mimes["woff2"] = "font/woff2";

    _mimes["jar"] = "application/java-archive";
    _mimes["war"] = "application/java-archive";
    _mimes["ear"] = "application/java-archive";
    _mimes["json"] = "application/json";
    _mimes["hqx"] = "application/mac-binhex40";
    _mimes["doc"] = "application/msword";
    _mimes["pdf"] = "application/pdf";
    _mimes["ps"] = "application/postscript";
    _mimes["eps"] = "application/postscript";
    _mimes["ai"] = "application/postscript";
    _mimes["rtf"] = "application/rtf";
    _mimes["m3u8"] = "application/vnd.apple.mpegurl";
    _mimes["kml"] = "application/vnd.google-earth.kml+xml";
    _mimes["kmz"] = "application/vnd.google-earth.kmz";
    _mimes["xls"] = "application/vnd.ms-excel";
    _mimes["eot"] = "application/vnd.ms-fontobject";
    _mimes["ppt"] = "application/vnd.ms-powerpoint";
    _mimes["odg"] = "application/vnd.oasis.opendocument.graphics";
    _mimes["odp"] = "application/vnd.oasis.opendocument.presentation";
    _mimes["ods"] = "application/vnd.oasis.opendocument.spreadsheet";
    _mimes["odt"] = "application/vnd.oasis.opendocument.text";

    _mimes["pptx"] = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
    _mimes["xlsx"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
    _mimes["docx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";

    _mimes["wmlc"] = "application/vnd.wap.wmlc";
    _mimes["7z"] = "application/x-7z-compressed";
    _mimes["cco"] = "application/x-cocoa";
    _mimes["jardiff"] = "application/x-java-archive-diff";
    _mimes["jnlp"] = "application/x-java-jnlp-file";
    _mimes["run"] = "application/x-makeself";
    _mimes["pl"] = "application/x-perl";
    _mimes["pm"] = "application/x-perl";
    _mimes["prc"] = "application/x-pilot";
    _mimes["pdb"] = "application/x-pilot";
    _mimes["rar"] = "application/x-rar-compressed";
    _mimes["rpm"] = "application/x-redhat-package-manager";
    _mimes["sea"] = "application/x-sea";
    _mimes["swf"] = "application/x-shockwave-flash";
    _mimes["sit"] = "application/x-stuffit";
    _mimes["tcl"] = "application/x-tcl";
    _mimes["tk"] = "application/x-tcl";
    _mimes["der"] = "application/x-x509-ca-cert";
    _mimes["pem"] = "application/x-x509-ca-cert";
    _mimes["crt"] = "application/x-x509-ca-cert";
    _mimes["xpi"] = "application/x-xpinstall";
    _mimes["xhtml"] = "application/xhtml+xml";
    _mimes["xspf"] = "application/xspf+xml";
    _mimes["zip"] = "application/zip";

    _mimes["bin"] = "application/octet-stream";
    _mimes["exe"] = "application/octet-stream";
    _mimes["dll"] = "application/octet-stream";
    _mimes["deb"] = "application/octet-stream";
    _mimes["dmg"] = "application/octet-stream";
    _mimes["iso"] = "application/octet-stream";
    _mimes["img"] = "application/octet-stream";
    _mimes["msi"] = "application/octet-stream";
    _mimes["msp"] = "application/octet-stream";
    _mimes["msm"] = "application/octet-stream";

    _mimes["mid"] = "audio/midi";
    _mimes["midi"] = "audio/midi";
    _mimes["kar"] = "audio/midi";
    _mimes["mp3"] = "audio/mpeg";
    _mimes["ogg"] = "audio/ogg";
    _mimes["m4a"] = "audio/x-m4a";
    _mimes["ra"] = "audio/x-realaudio";

    _mimes["3gpp"] = "video/3gpp";
    _mimes["3gp"] = "video/3gpp";
    _mimes["ts"] = "video/mp2t";
    _mimes["mp4"] = "video/mp4";
    _mimes["mpeg"] = "video/mpeg";
    _mimes["mpg"] = "video/mpeg";
    _mimes["mov"] = "video/quicktime";
    _mimes["webm"] = "video/webm";
    _mimes["flv"] = "video/x-flv";
    _mimes["m4v"] = "video/x-m4v";
    _mimes["mng"] = "video/x-mng";
    _mimes["asx"] = "video/x-ms-asf";
    _mimes["asf"] = "video/x-ms-asf";
    _mimes["wmv"] = "video/x-ms-wmv";
    _mimes["avi"] = "video/x-msvideo";

    _set_mimes.insert("text/html");
    _set_mimes.insert("text/css");
    _set_mimes.insert("text/xml");
    _set_mimes.insert("image/gif");
    _set_mimes.insert("image/jpeg");
    _set_mimes.insert("application/javascript");
    _set_mimes.insert("application/javascript");
    _set_mimes.insert("application/atom+xml");
    _set_mimes.insert("application/rss+xml");

    _set_mimes.insert("text/mathml");
    _set_mimes.insert("text/plain");
    _set_mimes.insert("text/vnd.sun.j2me.app-descriptor");
    _set_mimes.insert("text/vnd.wap.wml");
    _set_mimes.insert("text/x-component");

    _set_mimes.insert("image/png");
    _set_mimes.insert("image/svg+xml");
    _set_mimes.insert("image/svg+xml");
    _set_mimes.insert("image/tiff");
    _set_mimes.insert("image/tiff");
    _set_mimes.insert("image/vnd.wap.wbmp");
    _set_mimes.insert("image/webp");
    _set_mimes.insert("image/x-icon");
    _set_mimes.insert("image/x-jng");
    _set_mimes.insert("image/x-ms-bmp");

    _set_mimes.insert("font/woff");
    _set_mimes.insert("font/woff2");

    _set_mimes.insert("application/java-archive");
    _set_mimes.insert("application/java-archive");
    _set_mimes.insert("application/java-archive");
    _set_mimes.insert("application/json");
    _set_mimes.insert("application/mac-binhex40");
    _set_mimes.insert("application/msword");
    _set_mimes.insert("application/pdf");
    _set_mimes.insert("application/postscript");
    _set_mimes.insert("application/postscript");
    _set_mimes.insert("application/postscript");
    _set_mimes.insert("application/rtf");
    _set_mimes.insert("application/vnd.apple.mpegurl");
    _set_mimes.insert("application/vnd.google-earth.kml+xml");
    _set_mimes.insert("application/vnd.google-earth.kmz");
    _set_mimes.insert("application/vnd.ms-excel");
    _set_mimes.insert("application/vnd.ms-fontobject");
    _set_mimes.insert("application/vnd.ms-powerpoint");
    _set_mimes.insert("application/vnd.oasis.opendocument.graphics");
    _set_mimes.insert("application/vnd.oasis.opendocument.presentation");
    _set_mimes.insert("application/vnd.oasis.opendocument.spreadsheet");
    _set_mimes.insert("application/vnd.oasis.opendocument.text");

    _set_mimes.insert("application/vnd.openxmlformats-officedocument.presentationml.presentation");
    _set_mimes.insert("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet");
    _set_mimes.insert("application/vnd.openxmlformats-officedocument.wordprocessingml.document");

    _set_mimes.insert("application/vnd.wap.wmlc");
    _set_mimes.insert("application/x-7z-compressed");
    _set_mimes.insert("application/x-cocoa");
    _set_mimes.insert("application/x-java-archive-diff");
    _set_mimes.insert("application/x-java-jnlp-file");
    _set_mimes.insert("application/x-makeself");
    _set_mimes.insert("application/x-perl");
    _set_mimes.insert("application/x-perl");
    _set_mimes.insert("application/x-pilot");
    _set_mimes.insert("application/x-pilot");
    _set_mimes.insert("application/x-rar-compressed");
    _set_mimes.insert("application/x-redhat-package-manager");
    _set_mimes.insert("application/x-sea");
    _set_mimes.insert("application/x-shockwave-flash");
    _set_mimes.insert("application/x-stuffit");
    _set_mimes.insert("application/x-tcl");
    _set_mimes.insert("application/x-tcl");
    _set_mimes.insert("application/x-x509-ca-cert");
    _set_mimes.insert("application/x-x509-ca-cert");
    _set_mimes.insert("application/x-x509-ca-cert");
    _set_mimes.insert("application/x-xpinstall");
    _set_mimes.insert("application/xhtml+xml");
    _set_mimes.insert("application/xspf+xml");
    _set_mimes.insert("application/zip");

    _set_mimes.insert("application/octet-stream");
    _set_mimes.insert("application/octet-stream");
    _set_mimes.insert("application/octet-stream");
    _set_mimes.insert("application/octet-stream");
    _set_mimes.insert("application/octet-stream");
    _set_mimes.insert("application/octet-stream");
    _set_mimes.insert("application/octet-stream");
    _set_mimes.insert("application/octet-stream");
    _set_mimes.insert("application/octet-stream");
    _set_mimes.insert("application/octet-stream");

    _set_mimes.insert("audio/midi");
    _set_mimes.insert("audio/midi");
    _set_mimes.insert("audio/midi");
    _set_mimes.insert("audio/mpeg");
    _set_mimes.insert("audio/ogg");
    _set_mimes.insert("audio/x-m4a");
    _set_mimes.insert("audio/x-realaudio");

    _set_mimes.insert("video/3gpp");
    _set_mimes.insert("video/3gpp");
    _set_mimes.insert("video/mp2t");
    _set_mimes.insert("video/mp4");
    _set_mimes.insert("video/mpeg");
    _set_mimes.insert("video/mpeg");
    _set_mimes.insert("video/quicktime");
    _set_mimes.insert("video/webm");
    _set_mimes.insert("video/x-flv");
    _set_mimes.insert("video/x-m4v");
    _set_mimes.insert("video/x-mng");
    _set_mimes.insert("video/x-ms-asf");
    _set_mimes.insert("video/x-ms-asf");
    _set_mimes.insert("video/x-ms-wmv");
    _set_mimes.insert("video/x-msvideo");
}

//int    Host::get_load(Worker* w) {return _worker_load[w];}
int    Host::get_max_clients(void) const {return (_max_clients);}
int    Host::get_n_workers(void) const {return (_n_workers);}
int    Host::get_timeout(void) const {return (_timeout);}
size_t Host::get_large_client_header_buffer(void) const
{
    return (_large_client_header_buffer);
}
size_t Host::get_client_max_body_size(void) const
{
    return (_client_max_body_size);
}
size_t Host::get_client_body_buffer_size(void) const
{
    return (_client_body_buffer_size);
}
int  Host::get_stop(void)
{
    int t;
    pthread_mutex_lock(&_stop_mutex);
    t = _stop;
    pthread_mutex_unlock(&_stop_mutex);
    return (t);
}
std::map<std::string, std::string>*	Host::get_mimes(void) {return (&_mimes);}
std::set<std::string>*	    Host::get_set_mimes(void) {return (&_set_mimes);}
std::map<int, std::string>* Host::get_status_message(void) {return (&_status_message);}
Worker*		 Host::get_workers(void) const {return (_workers);}
pthread_mutex_t* Host::get_cout_mutex(void) {return (&_cout_mutex);}
pthread_mutex_t* Host::get_fd_mutex(void) {return (&_fd_mutex);}
pthread_mutex_t* Host::get_sk_worker_mutex(void) {return (&_sk_worker_mutex);}

std::map<int, Address*>* Host::get_sk_address(void) {return (&_sk_address);}
std::map<int, Worker*>*	 Host::get_sk_worker(void) {return (&_sk_worker);}

void Host::set_client_max_body_size(size_t n) {_client_max_body_size = n;}
void Host::set_client_body_buffer_size(size_t n) {_client_body_buffer_size = n;}
void Host::set_parser_error(bool e) {_parser_error = e;}
void Host::set_str_address(std::map<std::string, Address*> a){_str_address = a;}
void Host::set_n_workers(int w) {_n_workers = w;}
void Host::set_large_client_header_buffer(size_t l)
{
    _large_client_header_buffer = l;
}
void Host::set_timeout(int t) {_timeout = t;}
void Host::set_stop(int t)
{
    pthread_mutex_lock(&_stop_mutex);
    _stop = t;
    pthread_mutex_unlock(&_stop_mutex);
}
