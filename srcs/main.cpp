/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nbechon <nbechon@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/05 21:21:18 by ngoc              #+#    #+#             */
/*   Updated: 2024/01/30 15:19:15 by nbechon          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <signal.h>

#include "Host.hpp"
#include "Worker.hpp"
#include "Server.hpp"
#include "Configuration.hpp"

Host	*g_host;

void	main_signal_handler(int sig)
{
	if (sig == SIGINT)
	{
        g_host->set_end(true);
        std::cout << "\b\b";
        Worker*     workers = g_host->get_workers();
        for (int i = 0; i < g_host->get_n_workers(); i++)
            workers[i].set_terminate_flag(true);
	}
	if (sig == SIGPIPE)	{}
}

int	main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << RED << "Use: ./webserv .conf" << RESET << std::endl;
        return (1);
    }
    struct sigaction	act;
    act.sa_flags = SA_RESTART;
    act.sa_handler = main_signal_handler;
    sigemptyset(&act.sa_mask);
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGPIPE, &act, NULL);
    Host	host;
    if (!Configuration::parser(&host, argv[1]))
        return (1);
    g_host = &host;
    host.start();
    return (0);
}
