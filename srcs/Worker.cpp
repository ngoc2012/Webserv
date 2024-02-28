/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Worker.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/02/24 22:39:34 by ngoc             ###   ########.fr       */
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
    pthread_mutex_init(&_terminate_mutex, NULL);
    pthread_mutex_init(&_set_mutex, NULL);
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
}

void	Worker::routine(void)
{
    std::map<int, Request*>::iterator next;
    for (std::map<int, Request*>::iterator it = _sk_request.begin(), next = it;
        it != _sk_request.end(); it = next)
    {
        next++;
        double  dt = static_cast<double>((0) - _sk_timeout[it->first]);
        if (dt > it->second->get_timeout())
        {
            ft::timestamp();
            std::cout << MAGENTA << "Time Out " << it->first << "(" << dt << ")" << RESET << std::endl;
            
            close_client_sk(it->first);
            continue;
        }
        pthread_mutex_lock(&_set_mutex);
        if (FD_ISSET(it->first, &_read_set) && !_sk_request[it->first]->get_end())
        {
            pthread_mutex_unlock(&_set_mutex);
            if (_sk_request[it->first]->read() < 0)
                close_client_sk(it->first);
        }
        else if (FD_ISSET(it->first, &_write_set) && _sk_request[it->first]->get_end())
        {
            pthread_mutex_unlock(&_set_mutex);
            if (_sk_request[it->first]->get_response()->write() < 0)
                close_client_sk(it->first);
        }
        else
            pthread_mutex_unlock(&_set_mutex);
    }
}

void	Worker::set_empty_sets(void)
{
    _workload = 0;
    FD_ZERO(&_tmp_read_set);
    FD_ZERO(&_tmp_write_set);
}

void	Worker::set_sk_tmp_read_set(int sk)
{
    FD_SET(sk, &_tmp_read_set);
    if (_sk_request.find(sk) != _sk_request.end() && !_sk_request[sk]->get_end())
        _workload++;
}

void	Worker::set_sk_tmp_write_set(int sk)
{
    FD_SET(sk, &_tmp_write_set);
    if (_sk_request.find(sk) != _sk_request.end() && _sk_request[sk]->get_end())
        _workload++;
}

void	Worker::update_sets(void)
{
    pthread_mutex_lock(&_set_mutex);
    memcpy(&_read_set, &_tmp_read_set, sizeof(_tmp_read_set));
    memcpy(&_write_set, &_tmp_write_set, sizeof(_tmp_write_set));
    pthread_mutex_unlock(&_set_mutex);
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
int          Worker::get_workload(void) {return (_workload);}
pthread_mutex_t*    Worker::get_terminate_mutex(void) {return (&_terminate_mutex);}
bool				Worker::get_terminate_flag(void) const {return (_terminate_flag);}

void         Worker::set_id(int s) {_id = s;}
void         Worker::set_workload(int s) {_workload = s;}
void         Worker::set_host(Host* h) {_host = h;}
// void	     Worker::set_timeout(int t) {_timeout = t;}
void	     Worker::set_terminate_mutex(pthread_mutex_t m) {_terminate_mutex = m;}
void	     Worker::set_terminate_flag(bool f)
{
    pthread_mutex_lock(&_terminate_mutex);
    _terminate_flag = f;
    pthread_mutex_unlock(&_terminate_mutex);
}