/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Listing.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2023/12/31 10:23:49 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>

#ifndef LISTING_HPP
# define LISTING_HPP

class   Response;

class	Listing
{
	private:
		Listing(const Listing&);
		Listing	&operator=(const Listing& op);

	public:
		Listing();
		virtual ~Listing();

        static std::string get_html(Response*);
		static std::string get_listing(const std::string& directory_name, const std::string& base_path = "");
};

#endif
