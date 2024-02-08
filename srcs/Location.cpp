/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nbechon <nbechon@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/01/30 16:26:28 by nbechon          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Location.hpp"
#include "Host.hpp"

Location::Location()
{
    _autoindex = false;
    _cgi_pass = "";
	_redirection = 0;
}
Location::Location(const Location& src) { *this = src; }
Location&	Location::operator=( Location const & src )
{
	(void) src;
	return (*this);
}
Location::Location(std::string u): _url(u)
{
    _autoindex = false;
    _cgi_pass = "";
	_redirection = 0;
}
Location::~Location() {}


Location*	Location::find_location(std::string url, std::vector<Location*> locations, e_method method, int &status_code)
{
    Location*   loc = 0;

	for (std::vector<Location*>::iterator it = locations.begin();
		it != locations.end(); ++it)
	{
        ///std::cout << url << ":" << (*it)->get_url() << std::endl;
		if ((*it)->compare_url(url, (*it)->get_url()))
		{
			if ((*it)->find_method(method))
				status_code = 200;
			else
            {
                //std::cerr << "Method not allowed\n";
				status_code = 405; // Method not allowed
            }
            loc = *it;
		}
	}
    if (!loc)
        status_code = 404; // Not found
    return (loc);
}

bool	Location::compare_url(std::string url, std::string l_url)
{
	//std::cout << url << "==" << l_url << std::endl;
	if (url == l_url)
		return (true);
    if (_url.size() < url.size() && _url == url.substr(0, _url.size()))
        return (true);
    if (l_url.find('*') != NPOS
            && ft::match_wildcard(url.c_str(), l_url.c_str()))
		return (true);
	return (false);
}

bool	Location::find_method(e_method m)
{
	for (std::vector<e_method>::iterator	it = _methods.begin();
		it != _methods.end(); ++it)
	{
		if (m == *it)
			return (true);
    }
	return (false);
}

std::string	Location::get_full_file_name(std::string url, std::string root, e_method e)
{
    std::string file_name;

    if (_alias == "")
        file_name = root;
    else
        file_name = _alias;
    if (_url.size() < url.size())
        file_name += url.substr(1);
    if (_autoindex || e == PUT || e == POST)
        return (file_name);
    struct stat	info;
    if (stat(file_name.c_str(), &info))
        return (file_name);
    if (!S_ISDIR(info.st_mode))
        return (file_name);
    if (!_index.size())
    {
        file_name += "index.html";
        return (file_name);
    }
    long unsigned int     i = 0;
    std::string fn0 = file_name + _index[i];
    while (stat(fn0.c_str(), &info) && ++i < _index.size())
        fn0 = file_name + _index[i];
    return (fn0);
}

std::string	Location::get_methods_str(void)
{
	std::string	s = "";
	for(std::vector<e_method>::iterator it = _methods.begin();
            it != _methods.end(); ++it)
	{
		if (it != _methods.begin())
			s += ",";
        s += get_method_str(*it);
	}
	return (s);
}

std::string	Location::get_method_str(e_method e) {
    switch (e)
    {
		case OPTIONS:
			return ("OPTIONS");
        case HEAD:
			return ("HEAD");
        case GET:
            return ("GET");
        case POST:
            return ("POST");
        case DELETE:
            return ("DELETE");
        case PUT:
            return ("PUT");
        case NONE:
            return ("UNKNOWN");
    }
    return ("");
}

// bool    Location::verifieExtension(const std::string& nomFichier) {
//     std::string extension = nomFichier.substr(nomFichier.length() - 5);
//     return (extension == "*.bla");
// }

void        Location::push_back_index(std::string s) { _index.push_back(s); }

std::vector<e_method>		Location::get_methods(void) const {return (_methods);}
std::string			        Location::get_alias(void) const {return (_alias);}
std::string			        Location::get_url(void) const {return (_url);}
std::string			        Location::get_cgi_pass(void) const {return (_cgi_pass);}
bool                        Location::get_autoindex(void) const {return (_autoindex);}
int							Location::get_redirection(void) const {return (_redirection);}
std::string			        Location::get_link(void) const {return (_link);}

void				Location::insert_methods(e_method e) {_methods.push_back(e);}
void				Location::set_alias(std::string s) {_alias = s;}
void				Location::set_url(std::string u) {_url = u;}
void			    Location::set_cgi_pass(std::string c) {_cgi_pass = c;}
void                Location::set_autoindex(bool a) {_autoindex = a;}
void				Location::set_redirection(int r) {_redirection = r;}
void                Location::set_link(std::string l) {_link = l;}
