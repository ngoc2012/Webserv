/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/05 21:21:18 by ngoc              #+#    #+#             */
/*   Updated: 2024/01/04 15:46:05 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <signal.h>

#include "Host.hpp"
#include "Server.hpp"
#include "Configuration.hpp"

Host	*g_host;

void	main_signal_handler(int sig)
{
	if (sig == SIGINT)
	{
		write(STDOUT_FILENO, "", 0);
		if (g_host)
			delete g_host;
		exit(0);
	}
	if (sig == SIGPIPE)	{}
}

int	main()
{
    struct sigaction	act;
    act.sa_flags = SA_RESTART;
    act.sa_handler = main_signal_handler;
    sigemptyset(&act.sa_mask);
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGPIPE, &act, NULL);

    Host*	host = new Host();
    if (!Configuration::parser(host, ".conf"))
        return (1);
    g_host = host;
    host->start();
    return (0);
}
