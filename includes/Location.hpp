/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/01/27 22:20:58 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <vector>

#include "webserv.hpp"

#ifndef LOCATION_HPP
# define LOCATION_HPP

class	Location
{
	private:
		std::string			        _url;
		std::vector<e_method>	    _methods;
		std::string			        _alias;
		std::string			        _cgi_pass;
		bool                        _autoindex;
		std::vector<std::string>	_index;
		int							_redirection;
		std::string			        _link;


		Location();
		Location(const Location&);
		Location			&operator=(const Location& op);
	public:
		Location(std::string);
		virtual ~Location();

        static Location*	        find_location(std::string, std::vector<Location*>, e_method, int &);
		bool	                    compare_url(std::string, std::string);
		bool		                find_method(e_method);
        std::string	                get_full_file_name(std::string, std::string, e_method);
		std::string                 get_methods_str(void);
        static std::string	        get_method_str(e_method);
		void                        push_back_index(std::string);

		std::vector<e_method>		get_methods(void) const;
		std::string			        get_alias(void) const;
		std::string			        get_url(void) const;
        std::string			        get_cgi_pass(void) const;
        bool                        get_autoindex(void) const;
        int							get_redirection(void) const;
        std::string			        get_link(void) const;

		void				        insert_methods(e_method);
		void				        set_alias(std::string);
		void				        set_url(std::string);
        void			            set_cgi_pass(std::string);
        void                        set_autoindex(bool);
        void						set_redirection(int);
        void                        set_link(std::string);
};

#endif
