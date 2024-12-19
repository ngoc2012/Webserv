/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Worker.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/03/05 11:07:46 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include <iostream>
#include <pthread.h>

#include "webserv.hpp"

#ifndef WORKER_HPP
# define WORKER_HPP

class	Host;
class	Address;
class	Server;
class	Request;
class	Response;
class	Configuration;

class	Worker
{
	private:
        
		int         _id;
		//int         _workload;  // Workload metric
		pthread_t   _th;
	
		Host*       _host;
		int			_sk_ready;
		int			_max_sk;		    // Max of all fd
		
		fd_set      _tmp_read_set;		// Set of active read fd
		fd_set      _tmp_write_set;		// Set of active write fd
		fd_set      _read_set;		    // Set of active read fd
		fd_set      _write_set;		    // Set of active write fd
		volatile bool		_terminate_flag;

		std::map<int, Request*>		_sk_request;
		std::map<int, time_t>		_sk_timeout;
		std::map<int, Address*>		_sk_address;

		pthread_mutex_t		_set_mutex;
		pthread_mutex_t		_terminate_mutex;
		//pthread_mutex_t		_workload_mutex;
		pthread_mutex_t		_sk_size_mutex;
		pthread_mutex_t		_timeout_mutex;
		//pthread_mutex_t		_set_updated_mutex;
		pthread_cond_t		_cond_set_updated;
		bool				_set_updated;

		Worker(const Worker&);
		Worker	&operator=(const Worker& op);

	public:
		Worker();
		virtual ~Worker();

		void    	routine(void);

		void		new_connection(int, Address*);
		void		close_client_sk(int);
		void		set_sk_timeout(int);
		void		set_empty_sets(void);
		void		set_sk_tmp_read_set(int);
		void		set_sk_tmp_write_set(int);
		void		update_sets(void);
		void		set_end(void);
		void		check_timeout(void);
		void		need_host_update_set(void);
		void		wait_for_set_updated(void);

        pthread_t*  		get_th(void);
        int         		get_id(void) const;
        /int         		get_workload(void);
        Host*       		get_host(void) const;
        pthread_mutex_t*	get_terminate_mutex(void);
        pthread_mutex_t*	get_set_mutex(void);
		//pthread_mutex_t*	get_set_updated_mutex(void);
        pthread_cond_t*		get_cond_set_updated(void);
		bool				get_terminate_flag(void);
        bool				get_set_updated(void);
        std::map<int, Request*>*		get_sk_request(void);

        void        set_id(int);
        //void        set_workload(int);
        void        set_host(Host*);
		void		set_terminate_mutex(pthread_mutex_t);
		void		set_terminate_flag(bool);
        void		set_set_updated(bool);
};

#endif
