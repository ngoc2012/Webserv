/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Address.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/01/10 10:08:55 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Host.hpp"
#include "Server.hpp"
#include "webserv.hpp"

#include "Address.hpp"

Address::Address() {}
Address::Address(const Address& src) { *this = src; }
Address&	Address::operator=( Address const & src )
{
	(void) src;
	return (*this);
}
Address::~Address()
{
	for (std::vector<Server*>::iterator it = _servers.begin();
		it != _servers.end(); ++it)
		delete (*it);
	if (_listen_socket > 0)
		close(_listen_socket);
}
Address::Address(Host* host, std::string ip, short unsigned int p):
    _host(host),
    _ip_address(ip),
    _port(p)
{
	_listen_socket = -1;
}

void    Address::push(Server* s) { _servers.push_back(s); }

int     Address::listen_socket(void)
{
	_listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_listen_socket < 0)
	{
        std::cout << "Error: listen_socket: socket() failed " << _ip_address << ":" << _port << std::endl;
		return (-1);
	}
	int    on = 1;
	if (setsockopt(_listen_socket, SOL_SOCKET,  SO_REUSEADDR,
                   (char *)&on, sizeof(on)) < 0)
	{
        std::cout << "Error: listen_socket: setsockopt() failed " << _ip_address << ":" << _port << std::endl;
		return (-1);
	}
	fcntl(_listen_socket, F_SETFL, O_NONBLOCK);
	return (bind_addr());
}

int	    Address::bind_addr()
{
	struct sockaddr_in	addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(_port);
	addr.sin_addr.s_addr = inet_addr(_ip_address.c_str());
    //std::cout << _ip_address << ":" << _port << std::endl;
	if (bind(_listen_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		ft::timestamp();
        std::cout << "Error: bind_addr: bind() failed " << _ip_address << ":" << _port << std::endl;
		return (-1);
	}
	if (listen(_listen_socket, _host->get_max_clients()) < 0)
	{
		ft::timestamp();
        std::cout << "Error: bind_addr: listen() failed " << _ip_address << ":" << _port << std::endl;
		return (-1);
	}
	ft::timestamp();
	std::cout << "Listening at " << _ip_address << ":" << _port
		<< " (socket : " << _listen_socket << ")" << std::endl;
	return (_listen_socket);
}

//Accept all the new connections, create a new socket and add to the master set
void	Address::accept_client_sk(void)
{
	std::cout << "Listening socket is readable " << _listen_socket << std::endl;
	int	new_sk;
	//do
	//{
		new_sk = accept(_listen_socket, NULL, NULL);
		if (new_sk < 0)
		{
			ft::timestamp();
			std::cerr << "Error: accept function." << std::endl;
			//break;
		}
		fcntl(new_sk, F_SETFL, O_NONBLOCK);
		ft::timestamp();
		std::cout << "accept_client_sk " << new_sk << std::endl;
		_host->new_request_sk(new_sk, this);
	//} while (new_sk != -1);
}

std::vector<Server*>    Address::get_servers(void) {return (_servers);}