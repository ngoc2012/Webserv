/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Host.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/03/06 20:08:40 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <set>
#include <map>

#include "webserv.hpp"

#ifndef HOST_HPP
# define HOST_HPP

// Forward declaration
class	Worker;
class	Address;
class	Server;
class	Request;
class	Response;
class	Configuration;

class	Host
{
	private:

		int				                        _max_clients;
		size_t				                    _client_max_body_size;
		size_t				                    _client_body_buffer_size;
		size_t									_large_client_header_buffer;
	
		bool				                    _parser_error;
		bool									_end;
		int				                        _start_worker_id;
		int				                        _n_workers;
		Worker*				                    _workers;
		
		pthread_mutex_t							_cout_mutex;
		pthread_mutex_t							_set_mutex;
		pthread_mutex_t							_end_mutex;
		pthread_mutex_t							_fd_mutex;
		pthread_mutex_t							_sk_worker_mutex; // Worker should remove sk closed in sk_worker
		int										_timeout;

		int										_max_sk;		    // Max of all fd
		fd_set              		            _listen_set;
		fd_set      							_read_set;		    // Set of active read fd
		fd_set      							_write_set;		    // Set of active write fd
		fd_set      							_master_read_set;	// Set of all read socket
		fd_set      							_master_write_set;	// Set of all write socket
		
		std::map<std::string, Address*>		    _str_address;
		std::map<int, Worker*>		            _sk_worker;
		std::map<int, Address*>		            _sk_address;
		std::map<Worker*, int>		            _worker_load;
		
		std::map<int, std::string>  		    _status_message;
		std::map<std::string, std::string>	    _mimes;
		std::set<std::string>	                _set_mimes;
		
		void    								start_server(void);
		void				                    mimes(void);
		void				                    status_message(void);
		bool									start_workers(void);
		void    								check_sk_ready(void);
		
		Host(const Host&);
		Host &operator=(const Host& op);

	public:
		Host();
		virtual ~Host();

		void    		                    start(void);
		void  								close_connection(int);
		void  								insert_read_fd(int);
		void  								clear_read_fd(int);
		// void  								wait_for_fd_set_need_update(void);
		void  								print(e_message, std::string);
		void								round_robin(int, Address*);
		bool								is_writable_fd(int);
		bool								is_readable_fd(int);

		int				                    get_max_clients(void) const;
		size_t				                get_client_max_body_size(void) const;
		size_t				                get_client_body_buffer_size(void) const;
		int				                    get_load(Worker* w);

        std::map<std::string, std::string>*	get_mimes(void);
        std::set<std::string>*	            get_set_mimes(void);
        std::map<int, std::string>*  		get_status_message(void);
        Worker*				                get_workers(void) const;
		int					                get_n_workers(void) const;
        size_t								get_large_client_header_buffer(void) const;
		int									get_timeout(void) const;
        pthread_mutex_t*					get_cout_mutex(void);
        pthread_mutex_t*					get_end_mutex(void);
        pthread_mutex_t*					get_fd_mutex(void);
		// pthread_mutex_t*					get_sk_worker_mutex(void);
        bool								get_end(void);
		// pthread_mutex_t*					get_need_update_mutex(void);
		// pthread_cond_t*						get_cond_need_update(void);
		// bool								get_need_update(void) const;
		std::map<int, Address*>*			get_sk_address(void);
		std::map<int, Worker*>*				get_sk_worker(void);

		void	set_client_max_body_size(size_t);
		void	set_client_body_buffer_size(size_t);
		void	set_parser_error(bool);
        void    set_str_address(std::map<std::string, Address*>);
        void	set_n_workers(int);
        void	set_large_client_header_buffer(size_t);
		void	set_timeout(int);
		void	set_end(bool);
		// void	set_need_update(bool);
};

#endif
