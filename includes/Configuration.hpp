/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Configuration.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/01/04 15:43:53 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

#ifndef CONFIGURATIONPARSER_HPP
# define CONFIGURATIONPARSER_HPP

class Host;
class Address;
class Server;
class Location;

class Configuration
{
	private:
		static bool			    listen(Server*, std::vector<std::string>);
		static void			    conf_file_error(std::string, int);
		static std::string		remove_comments(std::string&);
		static std::string		remove_spaces_end(std::string&);
		static bool			    location_parser(std::string, Location*, std::vector<std::string>&);
		static bool			    server_parser(std::string, Server*, std::vector<std::string>&);
		static bool			    host_parser(std::string, Host*, std::vector<std::string>&);
        static bool             add_server(Host*, Server*, std::map<std::string, Address*>&);

		Configuration(const Configuration&);
		Configuration	&operator=(const Configuration& op);
	public:
		Configuration();
		virtual			~Configuration();

		static bool     parser(Host*, const char* conf);
};

#endif
