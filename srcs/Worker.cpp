/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Worker.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/03/05 11:07:53 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ctime>

#include "Host.hpp"
#include "Address.hpp"
#include "Server.hpp"
#include "Response.hpp"
#include "Request.hpp"
#include "Configuration.hpp"

#include "Worker.hpp"

Worker::Worker()
{
    _id = -1;
    //_workload = 0;
    _terminate_flag = false;
    _set_updated = false;
    pthread_mutex_init(&_terminate_mutex, NULL);
    pthread_mutex_init(&_set_mutex, NULL);
    //pthread_mutex_init(&_workload_mutex, NULL);
    pthread_mutex_init(&_sk_size_mutex, NULL);
    pthread_mutex_init(&_timeout_mutex, NULL);
    pthread_cond_init(&_cond_set_updated, NULL);
    FD_ZERO(&_tmp_read_set);
    FD_ZERO(&_tmp_write_set);
    FD_ZERO(&_read_set);
    FD_ZERO(&_write_set);
}

Worker::~Worker()
{
    for (std::map<int, Request*>::iterator it = _sk_request.begin();
            it != _sk_request.end(); ++it)
        delete (it->second);
    pthread_mutex_destroy(&_terminate_mutex);
    pthread_mutex_destroy(&_set_mutex);
    //pthread_mutex_destroy(&_workload_mutex);
    pthread_mutex_destroy(&_sk_size_mutex);
    pthread_mutex_destroy(&_timeout_mutex);
    pthread_cond_destroy(&_cond_set_updated);
}

void	Worker::routine(void)
{
    Request*    request;
    Response*   response;
    bool        worked = false;
    int         ret;
    int         fd;
    //int         fd_out;

    pthread_mutex_lock(&_sk_size_mutex);
    std::map<int, Request*>     sk_request = _sk_request;
    pthread_mutex_unlock(&_sk_size_mutex);

    for (std::map<int, Request*>::iterator it = sk_request.begin();
        it != sk_request.end(); it++)
    {   
        fd = it->first;
        request = it->second;
        pthread_mutex_lock(&_set_mutex);
        //if (FD_ISSET(fd, &_read_set) && !request->get_end())
        if (!request->get_end() && _host->is_readable_fd(fd))
        {
            pthread_mutex_unlock(&_set_mutex);
            worked = true;
            ret = request->read();
            if (ret < 0 && RUPTURE != 0)
                close_client_sk(fd);
        }
        //else if (FD_ISSET(fd, &_write_set) && request->get_end())
	else if (request->get_end() && _host->is_writable_fd(fd))
        {
            pthread_mutex_unlock(&_set_mutex);
            response = request->get_response();
	    //fd_out = response->get_fd_out();
	    //if (fd_out != -1 && !FD_ISSET(fd_out, &_read_set))
	    //{
	    //    _host->print(WARNING, "No write fd for response " + ft::itos(fd_out));
	    //    std::cout << FD_ISSET(fd_out, &_master_read_set) << std::endl;
	    //    continue;		    
	    //}
            worked = true;
            ret = response->write();
            if ((ret < 0 && RUPTURE != 0) || (response->get_end() && request->get_close()))
                close_client_sk(fd);
        }
        else
            pthread_mutex_unlock(&_set_mutex);
    }
    if (!worked)
        usleep(DELAY);
}

void	Worker::check_timeout(void)
{
    pthread_mutex_lock(&_sk_size_mutex);
    std::map<int, Request*>     sk_request = _sk_request;
    pthread_mutex_unlock(&_sk_size_mutex);

    for (std::map<int, Request*>::iterator it = sk_request.begin(), next = it;
        it != sk_request.end(); it++)
    {
        next++;
        pthread_mutex_lock(&_timeout_mutex);
        double  dt = static_cast<double>(time(0) - _sk_timeout[it->first]);
        pthread_mutex_unlock(&_timeout_mutex);
        if (dt > it->second->get_timeout())
        {
            _host->print(WARNING, "Time Out " + ft::itos(it->first) + " (" + ft::itos(dt) + "/" + ft::itos(it->second->get_timeout()) + ") wk: " + ft::itos(_id));
            close_client_sk(it->first);
        }
    }
}

void	Worker::set_empty_sets(void)
{
    //pthread_mutex_lock(&_workload_mutex);
    //_workload = 0;
    //pthread_mutex_unlock(&_workload_mutex);
    FD_ZERO(&_tmp_read_set);
    FD_ZERO(&_tmp_write_set);
    pthread_mutex_lock(&_set_mutex);
    FD_ZERO(&_read_set);
    FD_ZERO(&_write_set);
    pthread_mutex_unlock(&_set_mutex);
}

void	Worker::set_sk_tmp_read_set(int sk)
{
    FD_SET(sk, &_tmp_read_set);
    //pthread_mutex_lock(&_workload_mutex);
    //_workload++;
    //pthread_mutex_unlock(&_workload_mutex);
}

void	Worker::set_sk_tmp_write_set(int sk)
{
    FD_SET(sk, &_tmp_write_set);
    //pthread_mutex_lock(&_workload_mutex);
    //_workload++;
    //pthread_mutex_unlock(&_workload_mutex);
}

void	Worker::update_sets(void)
{
    memcpy(&_read_set, &_tmp_read_set, sizeof(_tmp_read_set));
    memcpy(&_write_set, &_tmp_write_set, sizeof(_tmp_write_set));
}

void	Worker::new_connection(int new_sk, Address* a)
{
    set_sk_timeout(new_sk);
    pthread_mutex_lock(&_sk_size_mutex);
	_sk_request[new_sk] = new Request(new_sk, this, a);
    pthread_mutex_unlock(&_sk_size_mutex);
}

void	Worker::close_client_sk(int fd)
{   
    pthread_mutex_lock(&_set_mutex);
    if (FD_ISSET(fd, &_read_set))
    {
        pthread_mutex_unlock(&_set_mutex);
        pthread_mutex_lock(&_workload_mutex);
        _workload--;
        pthread_mutex_unlock(&_workload_mutex);
    }
    else
        pthread_mutex_unlock(&_set_mutex);
    pthread_mutex_lock(&_set_mutex);
    if (FD_ISSET(fd, &_write_set))
    {
        pthread_mutex_unlock(&_set_mutex);
        pthread_mutex_lock(&_workload_mutex);
        _workload--;
        pthread_mutex_unlock(&_workload_mutex);
    }
    else
        pthread_mutex_unlock(&_set_mutex);
    _host->close_connection(fd);
    pthread_mutex_lock(&_sk_size_mutex);
	delete (_sk_request[fd]);
	_sk_request.erase(fd);
    pthread_mutex_unlock(&_sk_size_mutex);
}

void	Worker::set_sk_timeout(int i)
{
    pthread_mutex_lock(&_timeout_mutex);
    _sk_timeout[i] = time(0);
    pthread_mutex_unlock(&_timeout_mutex);
}

void		Worker::set_end(void)
{
    set_terminate_flag(true);
    pthread_mutex_lock(&_set_mutex);
    _set_updated = true;
    pthread_cond_signal(&_cond_set_updated);
    pthread_mutex_unlock(&_set_mutex);
}

void		Worker::wait_for_set_updated(void)
{
    pthread_mutex_lock(&_set_mutex);
    while (!_set_updated)
        pthread_cond_wait(&_cond_set_updated, &_set_mutex);
    _set_updated = false;
    pthread_mutex_unlock(&_set_mutex);
}

pthread_t*   Worker::get_th(void) {return (&_th);}
int          Worker::get_id(void) const {return (_id);}
Host*        Worker::get_host(void) const {return (_host);}
int          Worker::get_workload(void)
{
    pthread_mutex_lock(&_workload_mutex);
    int     w = _workload;
    pthread_mutex_unlock(&_workload_mutex);
    return (w);
}
pthread_mutex_t*    Worker::get_terminate_mutex(void) {return (&_terminate_mutex);}
pthread_mutex_t*	Worker::get_set_mutex(void) {return (&_set_mutex);}
pthread_cond_t*		Worker::get_cond_set_updated(void) {return (&_cond_set_updated);}
std::map<int, Request*>*		Worker::get_sk_request(void) {return (&_sk_request);}

bool				Worker::get_set_updated(void)
{
    pthread_mutex_lock(&_set_mutex);
    bool updated = _set_updated;
    pthread_mutex_unlock(&_set_mutex);
    return (updated);
}

bool				Worker::get_terminate_flag(void)
{
    pthread_mutex_lock(&_terminate_mutex);
    bool     w = _terminate_flag;
    pthread_mutex_unlock(&_terminate_mutex);
    return (w);
}

void         Worker::set_id(int s) {_id = s;}
void         Worker::set_workload(int s) {_workload = s;}
void         Worker::set_host(Host* h) {_host = h;}
void	     Worker::set_terminate_mutex(pthread_mutex_t m) {_terminate_mutex = m;}
void		 Worker::set_set_updated(bool u)
{
    pthread_mutex_lock(&_set_mutex);
    _set_updated = u;
    pthread_mutex_unlock(&_set_mutex);
}

void	     Worker::set_terminate_flag(bool f)
{
    pthread_mutex_lock(&_terminate_mutex);
    _terminate_flag = f;
    pthread_mutex_unlock(&_terminate_mutex);
}
