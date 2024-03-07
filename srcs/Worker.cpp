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
    _workload = 0;
    _terminate_flag = false;
    _set_updated = false;
    pthread_mutex_init(&_terminate_mutex, NULL);
    pthread_mutex_init(&_set_mutex, NULL);
    pthread_mutex_init(&_workload_mutex, NULL);
    pthread_mutex_init(&_sk_size_mutex, NULL);
    pthread_mutex_init(&_timeout_mutex, NULL);
    //pthread_mutex_init(&_set_updated_mutex, NULL);
    pthread_cond_init(&_cond_set_updated, NULL);
    FD_ZERO(&_tmp_read_set);
    FD_ZERO(&_tmp_write_set);
    FD_ZERO(&_read_set);
    FD_ZERO(&_write_set);
}

Worker::~Worker()
{
    // std::cout << "Worker " << _id << " destructed" << std::endl;
    for (std::map<int, Request*>::iterator it = _sk_request.begin();
            it != _sk_request.end(); ++it)
        delete (it->second);
    pthread_mutex_destroy(&_terminate_mutex);
    pthread_mutex_destroy(&_set_mutex);
    pthread_mutex_destroy(&_workload_mutex);
    pthread_mutex_destroy(&_sk_size_mutex);
    pthread_mutex_destroy(&_timeout_mutex);
    //pthread_mutex_destroy(&_set_updated_mutex);
    pthread_cond_destroy(&_cond_set_updated);
}

void	Worker::routine(void)
{
    Request*    request;
    Response*   response;
    bool        worked = false;

    pthread_mutex_lock(&_sk_size_mutex);
    std::map<int, Request*>     sk_request = _sk_request;
    pthread_mutex_unlock(&_sk_size_mutex);

    for (std::map<int, Request*>::iterator it = sk_request.begin();
        it != sk_request.end(); it++)
    {   
        request = it->second;
        pthread_mutex_lock(&_set_mutex);
        if (FD_ISSET(it->first, &_read_set) && !request->get_end())
        {
            pthread_mutex_unlock(&_set_mutex);
            worked = true;
            request->read();
        }
        else if (FD_ISSET(it->first, &_write_set) && request->get_end())
        {
            pthread_mutex_unlock(&_set_mutex);
            worked = true;
            response = it->second->get_response();
            response->write();
            if (response->get_end() && request->get_close())
                close_client_sk(it->first);
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
    // bool        to = false;
    for (std::map<int, Request*>::iterator it = sk_request.begin(), next = it;
        it != sk_request.end(); it++)
    {
        next++;
        pthread_mutex_lock(&_timeout_mutex);
        double  dt = static_cast<double>(time(0) - _sk_timeout[it->first]);
        pthread_mutex_unlock(&_timeout_mutex);
        if (dt > it->second->get_timeout())
        {
            // to = true;
            pthread_mutex_lock(_host->get_cout_mutex());
            ft::timestamp();
            std::cout << MAGENTA << "Time Out " << it->first << " (" << dt << "/" << it->second->get_timeout() << ") wk: " << _id << RESET << std::endl;
            pthread_mutex_unlock(_host->get_cout_mutex());
            close_client_sk(it->first);
        }
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
    pthread_mutex_lock(&_workload_mutex);
    _workload++;
    pthread_mutex_unlock(&_workload_mutex);
}

void	Worker::set_sk_tmp_write_set(int sk)
{
    FD_SET(sk, &_tmp_write_set);
    pthread_mutex_lock(&_workload_mutex);
    _workload++;
    pthread_mutex_unlock(&_workload_mutex);
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

void	Worker::close_client_sk(int i)
{   
    pthread_mutex_lock(&_set_mutex);
    if (FD_ISSET(i, &_read_set))
    {
        pthread_mutex_unlock(&_set_mutex);
        pthread_mutex_lock(&_workload_mutex);
        _workload--;
        pthread_mutex_unlock(&_workload_mutex);
    }
    else
        pthread_mutex_unlock(&_set_mutex);
    pthread_mutex_lock(&_set_mutex);
    if (FD_ISSET(i, &_write_set))
    {
        pthread_mutex_unlock(&_set_mutex);
        pthread_mutex_lock(&_workload_mutex);
        _workload--;
        pthread_mutex_unlock(&_workload_mutex);
    }
    else
        pthread_mutex_unlock(&_set_mutex);
    _host->close_connection(i);
    pthread_mutex_lock(&_sk_size_mutex);
	delete (_sk_request[i]);
	_sk_request.erase(i);
    pthread_mutex_unlock(&_sk_size_mutex);

    pthread_mutex_t*    need_update_mutex = _host->get_need_update_mutex();

    pthread_mutex_lock(need_update_mutex);
    _host->set_need_update(true);
    pthread_cond_signal(_host->get_cond_need_update());
    pthread_mutex_unlock(need_update_mutex);
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
//pthread_mutex_t*	Worker::get_set_updated_mutex(void) {return (&_set_updated_mutex);}
pthread_cond_t*		Worker::get_cond_set_updated(void) {return (&_cond_set_updated);}
std::map<int, Request*>*		Worker::get_sk_request(void) {return (&_sk_request);}
bool				Worker::get_set_updated(void) const {return (_set_updated);}

bool				Worker::get_terminate_flag(void)
{
    pthread_mutex_lock(&_terminate_mutex);
    bool     w = _terminate_flag;
    pthread_mutex_unlock(&_terminate_mutex);
    return (w);
}

int         		Worker::get_sk_size(void)
{
    pthread_mutex_lock(&_sk_size_mutex);
    int     w = _sk_request.size();
    pthread_mutex_unlock(&_sk_size_mutex);
    return (w);
}

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
