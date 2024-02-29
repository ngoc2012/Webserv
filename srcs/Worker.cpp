/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Worker.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/02/29 21:36:49 by ngoc             ###   ########.fr       */
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
    _workload = 0;
    _terminate_flag = false;
    _set_updated = false;
    pthread_mutex_init(&_terminate_mutex, NULL);
    pthread_mutex_init(&_set_mutex, NULL);
    pthread_mutex_init(&_workload_mutex, NULL);
    pthread_mutex_init(&_sk_size_mutex, NULL);
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
    pthread_mutex_destroy(&_workload_mutex);
    pthread_mutex_destroy(&_sk_size_mutex);
    pthread_cond_destroy(&_cond_set_updated);
}

void	Worker::routine(void)
{
    Request*    request;
    Response*   response;

    std::map<int, Request*>::iterator next;
    for (std::map<int, Request*>::iterator it = _sk_request.begin(), next = it;
        it != _sk_request.end(); it = next)
    {
        next++;
        double  dt = static_cast<double>(time(0) - _sk_timeout[it->first]);
        if (dt > it->second->get_timeout())
        {
            pthread_mutex_lock(_host->get_cout_mutex());
            ft::timestamp();
            std::cout << MAGENTA << "Time Out " << it->first << " (" << dt << ")" << RESET << std::endl;
            pthread_mutex_unlock(_host->get_cout_mutex());
            close_client_sk(it->first);
            continue;
        }
        request = it->second;
        pthread_mutex_lock(&_set_mutex);
        if (FD_ISSET(it->first, &_read_set) && !request->get_end())
        {
            pthread_mutex_unlock(&_set_mutex);
            request->read();
        }
        else if (FD_ISSET(it->first, &_write_set) && request->get_end())
        {
            pthread_mutex_unlock(&_set_mutex);
            response = _sk_request[it->first]->get_response();
            response->write();
            if (response->get_end() && request->get_close())
                close_client_sk(it->first);
        }
        else
            pthread_mutex_unlock(&_set_mutex);
    }
}

void	Worker::set_empty_sets(void)
{
    pthread_mutex_lock(&_workload_mutex);
    _workload = 0;
    pthread_mutex_unlock(&_workload_mutex);
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
    //if (_sk_request.find(sk) != _sk_request.end() && !_sk_request[sk]->get_end())
    //{
        pthread_mutex_lock(&_workload_mutex);
        _workload++;
        pthread_mutex_unlock(&_workload_mutex);
    //}
}

void	Worker::set_sk_tmp_write_set(int sk)
{
    FD_SET(sk, &_tmp_write_set);
    //if (_sk_request.find(sk) != _sk_request.end() && _sk_request[sk]->get_end())
    //{
        pthread_mutex_lock(&_workload_mutex);
        _workload++;
        pthread_mutex_unlock(&_workload_mutex);
    //}
}

void	Worker::update_sets(void)
{
    memcpy(&_read_set, &_tmp_read_set, sizeof(_tmp_read_set));
    memcpy(&_write_set, &_tmp_write_set, sizeof(_tmp_write_set));
}

void	Worker::new_connection(int new_sk, Address* a)
{
    set_sk_timeout(new_sk);
	_sk_request[new_sk] = new Request(new_sk, this, a);
}

void	Worker::close_client_sk(int i)
{
    _workload -= 2;
    _host->close_connection(i);
	delete (_sk_request[i]);
	_sk_request.erase(i);
}

void	Worker::set_sk_timeout(int i)
{
    _sk_timeout[i] = time(0);
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
bool				Worker::get_set_updated(void) const {return (_set_updated);}
bool				Worker::get_terminate_flag(void) const {return (_terminate_flag);}
std::map<int, Request*>*		Worker::get_sk_request(void) {return (&_sk_request);}

void         Worker::set_id(int s) {_id = s;}
void         Worker::set_workload(int s) {_workload = s;}
void         Worker::set_host(Host* h) {_host = h;}
void	     Worker::set_terminate_mutex(pthread_mutex_t m) {_terminate_mutex = m;}
void		 Worker::set_set_updated(bool u) {_set_updated = u;}
void	     Worker::set_terminate_flag(bool f)
{
    pthread_mutex_lock(&_terminate_mutex);
    _terminate_flag = f;
    pthread_mutex_unlock(&_terminate_mutex);
}
