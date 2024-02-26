/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Host.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/02/24 10:15:55 by ngoc             ###   ########.fr       */
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
		int				                        _n_workers;
		Worker*				                    _workers;
		volatile bool							_terminate_flag;
		pthread_mutex_t							_terminate_mutex;
		pthread_mutex_t							_set_mutex;
		pthread_cond_t							_terminate_cond;
		double									_timeout;

		int										_max_sk;		    // Max of all fd
		fd_set              		            _listen_set;
		fd_set      							_read_set;		    // Set of active read fd
		fd_set      							_write_set;		    // Set of active write fd
		fd_set      							_master_read_set;	// Set of all read fd
		fd_set      							_master_write_set;	// Set of all read fd
		
		std::map<std::string, Address*>		    _str_address;
		std::map<int, Worker*>		            _sk_worker;
		std::map<int, Address*>		            _sk_address;
		
		std::map<int, std::string>  		    _status_message;
		std::map<std::string, std::string>	    _mimes;
		std::set<std::string>	                _set_mimes;
		
		//bool				                    check_servers_conf(void);
		void    								start_server(void);
		void				                    mimes(void);
		void				                    status_message(void);
		bool									start_workers(void);
		void  									add_sk_2_master_read_set(int);
		void									new_connections(void);
		void    								check_sk_ready(void);
		bool									select_available_sk(void);
		

		Host(const Host&);
		Host &operator=(const Host& op);

	public:
		Host();
		virtual ~Host();

		void    		                    start(void);
		void  								close_connection(int i);

		int				                    get_max_clients(void) const;
		std::map<int, Server*>		        get_sk_server(void) const;
		std::map<int, Request*>	            get_sk_request(void) const;
		std::map<int, Response*>	        get_sk_response(void) const;
		size_t				                get_client_max_body_size(void) const;
		size_t				                get_client_body_buffer_size(void) const;
        std::map<std::string, std::string>*	get_mimes(void);
        std::set<std::string>*	            get_set_mimes(void);
        std::map<int, std::string>*  		get_status_message(void);
        Worker*				                get_workers(void) const;
		int					                get_n_workers(void) const;
        bool								get_terminate_flag(void) const;
        pthread_mutex_t*					get_terminate_mutex(void);
        pthread_cond_t*						get_terminate_cond(void);
        size_t								get_large_client_header_buffer(void) const;
		int									get_timeout(void) const;

		void	set_client_max_body_size(size_t);
		void	set_client_body_buffer_size(size_t);
		void	set_parser_error(bool);
        void    set_str_address(std::map<std::string, Address*>);
        void	set_n_workers(int);
        void	set_terminate_flag(bool);
        void	set_terminate_mutex(pthread_mutex_t);
        void	set_large_client_header_buffer(size_t);
		void	set_timeout(int);
		void	set_end(bool);
};

#endif
