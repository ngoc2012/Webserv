/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Listing.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/01/06 12:11:25 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"
#include "Response.hpp"
#include "webserv.hpp"
#include <dirent.h>

#include "Listing.hpp"

Listing::Listing() {}

Listing::Listing(const Listing& src) { *this = src; }

Listing&	Listing::operator=( Listing const & src )
{
	(void) src;
	return (*this);
}

Listing::~Listing() {}

std::string Listing::get_listing(const std::string& directory_name, const std::string& base_path) {
    std::string listing_html = "<!DOCTYPE html>\n\
        <html>\n\
        <head>\n\
        <title>Listing</title>\n\
        <style>\n\
        .folder-link {\n\
            font-weight: bold;\n\
            color: blue;\n\
        }\n\
        </style>\n\
        </head>\n\
        <body>\n";
    
    std::string base_directory = directory_name.substr(directory_name.find_last_of("/\\") + 1);

    listing_html += "<h2>Contenu du repertoire : " + base_directory + "</h2>\n";
    listing_html += "<ul>\n";

    const char* directory_path = directory_name.c_str();
    DIR* directory = opendir(directory_path);

    if (directory) {
        struct dirent* entry;
        while ((entry = readdir(directory))) {
            if (entry->d_type == DT_REG) {
                listing_html += "<li><a href='";
                listing_html += base_path + "/" + entry->d_name;
                listing_html += "'>" + std::string(entry->d_name) + "</a></li>\n";
            } else if (entry->d_type == DT_DIR
                    && std::string(entry->d_name) != "."
                    && std::string(entry->d_name) != "..") {
                std::string folder_path = directory_name + "/" + entry->d_name;
                listing_html += "<li><a href='" + base_path + "/" + entry->d_name +  "/'><strong>";
                listing_html += entry->d_name; 
                listing_html += "</strong></a></li>\n";
            }
        }
        closedir(directory);
    } else {
        listing_html += "Impossible d'ouvrir le répertoire.";
    }
    listing_html += "</ul>\n</body>\n</html>";
    return listing_html;
}

std::string Listing::get_html(Response* response) {
	(void)response;
	Request* request = response->get_request();
	std::string folder_name = request->get_full_file_name();
	std::string listing = get_listing(folder_name, request->get_url());
	return listing;
}

