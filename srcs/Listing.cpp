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
#include <sys/stat.h>
#include <ctime>

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
        table {\n\
            border-collapse: collapse;\n\
            width: 100%;\n\
        }\n\
        th, td {\n\
            border: 1px solid black;\n\
            padding: 8px;\n\
            text-align: left;\n\
        }\n\
        th {\n\
            background-color: #f2f2f2;\n\
        }\n\
        </style>\n\
        </head>\n\
        <body>\n";
    
    std::string base_directory = directory_name.substr(directory_name.find_last_of("/\\") + 1);

    listing_html += "<h2>Contenu du repertoire : " + base_directory + "</h2>\n";
    listing_html += "<table>\n";
    listing_html += "<tr><th>Name</th><th>Size</th><th>Last Modified</th></tr>\n";

    if (directory_name != "/") {
        std::string parent_directory = directory_name.substr(0, directory_name.find_last_of("/\\"));
        if (parent_directory.empty())
            parent_directory = "/";
        listing_html += "<tr><td><a href='" + base_path + "../'>Parent Directory</a></td><td></td><td></td></tr>\n";
    }

    const char* directory_path = directory_name.c_str();
    DIR* directory = opendir(directory_path);

    if (directory) {
        struct dirent* entry;
        while ((entry = readdir(directory))) {
            if (entry->d_type == DT_REG) {
                std::string file_path = directory_name + "/" + entry->d_name;
                struct stat file_stat;
                if (stat(file_path.c_str(), &file_stat) == 0) {
                    char date_modified[100];
                    strftime(date_modified, sizeof(date_modified), "%Y-%m-%d %H:%M:%S", localtime(&file_stat.st_mtime));
                    listing_html += "<tr><td><a href='";
                    listing_html += base_path + "/" + entry->d_name;
                    listing_html += "'>" + std::string(entry->d_name) + "</a></td><td>" + ft::itos(file_stat.st_size) + " bytes</td><td>" + std::string(date_modified) + "</td></tr>\n";
                } else {
                    listing_html += "<tr><td>" + std::string(entry->d_name) + "</td><td></td><td></td></tr>\n";
                }
            } else if (entry->d_type == DT_DIR && std::string(entry->d_name) != "." && std::string(entry->d_name) != "..") {
                std::string folder_path = directory_name + "/" + entry->d_name;
                struct stat folder_stat;
                if (stat(folder_path.c_str(), &folder_stat) == 0) {
                    char date_modified[100];
                    strftime(date_modified, sizeof(date_modified), "%Y-%m-%d %H:%M:%S", localtime(&folder_stat.st_mtime));
                    listing_html += "<tr><td><a href='" + base_path + "/" + entry->d_name +  "/'><strong>";
                    listing_html += entry->d_name; 
                    listing_html += "</strong></a></td><td>" + ft::itos(folder_stat.st_size) + " bytes</td><td>" + std::string(date_modified) + "</td></tr>\n";
                } else {
                    listing_html += "<tr><td>" + std::string(entry->d_name) + "</td><td></td><td></td></tr>\n";
                }
            }
        }
        closedir(directory);
    } else {
        listing_html += "<tr><td colspan='3'>Impossible d'ouvrir le répertoire.</td></tr>\n";
    }
    listing_html += "</table>\n</body>\n</html>";
    return listing_html;
}



std::string Listing::get_html(Response* response) {
	(void)response;
	Request* request = response->get_request();
	std::string folder_name = request->get_full_file_name();
	std::string listing = get_listing(folder_name, request->get_url());
	return listing;
}


