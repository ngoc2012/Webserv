/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nbechon <nbechon@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/05 21:21:18 by ngoc              #+#    #+#             */
/*   Updated: 2024/03/01 14:46:34 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <signal.h>

#include "Host.hpp"
#include "Worker.hpp"
#include "Configuration.hpp"

Host	*g_host;

void	main_signal_handler(int sig)
{
	if (sig == SIGINT)
	{
        // g_host->set_end(true);
        g_host->set_stop(1);
        pthread_mutex_lock(g_host->get_cout_mutex());
        std::cout << "\b\b";
        pthread_mutex_unlock(g_host->get_cout_mutex());
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
    for (int i = 0; i < host.get_n_workers(); i++)
    {
        std::cout << "End worker " << i << std::endl;
        host.get_workers()[i].set_end();
    }
        
    for (int i = 0; i < host.get_n_workers(); i++)
        pthread_join(*(host.get_workers()[i].get_th()), NULL);
    return (0);
}
