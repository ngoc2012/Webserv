/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/02/29 23:17:44 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Host.hpp"
#include "Location.hpp"

Server::Server()
{
	_ip_address = std::string("127.0.0.1");
	_port = 4242;
	_socket = -1;
	_host = 0;
	_root = "";
	_timeout = TIMEOUT;
}
Server::Server(const Server& src) { *this = src; }
Server&	Server::operator=( Server const & src )
{
	_ip_address = std::string(src.get_ip_address());
	_port = src.get_port();
	_socket = src.get_socket();
	_host = src.get_host();
	_root = src.get_root();
	return (*this);
}
Server::Server(Host* host): _host(host)
{
	_ip_address = std::string("127.0.0.1");
	_port = 4242;
	_socket = -1;
	_root = "";
	_timeout = _host->get_timeout();
    
}
Server::~Server()
{
	for (std::vector<Location*>::iterator it = _locations.begin();
			it != _locations.end(); ++it)
		delete(*it);
	if (_socket > 0)
	{
		std::cout << "~Server: Close socket " << _socket << std::endl;
		close(_socket);
	}
}

void			            Server::insert_location(Location* l) {_locations.push_back(l);}

std::string		            Server::get_address(void) const {return (_address);}
const char*		            Server::get_ip_address(void) const {return (_ip_address.c_str());}
short unsigned int	        Server::get_port(void) const {return (_port);}
int			                Server::get_socket(void) const {return (_socket);}
Host*			            Server::get_host(void) const {return (_host);}
std::string		            Server::get_root(void) const {return (_root);}
std::set<std::string>*   	Server::get_server_names(void) {return (&_server_names);}
std::vector<Location*>	    Server::get_locations(void) const {return (_locations);}
int		                    Server::get_timeout(void) const {return (_timeout);}


void			Server::set_socket(int i) {_socket = i;}
void			Server::set_address(std::string a) {_address = a;}
void			Server::set_ip_address(std::string ip) {_ip_address = ip;}
void			Server::set_port(short unsigned int p) {_port = p;}
void			Server::set_root(std::string r) {_root = r;}
void			Server::set_host(Host* h) {_host = h;}
void			Server::set_server_name(std::string s) {_server_names.insert(s);}
Sessions*		Server::get_sessions(void) {return (&_sessions);}
void	        Server::set_timeout(int t) {_timeout = t;}
