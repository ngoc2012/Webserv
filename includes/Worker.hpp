/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Worker.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/17 15:57:07 by ngoc              #+#    #+#             */
/*   Updated: 2024/02/03 21:40:48 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <pthread.h>

#ifndef WORKER_HPP
# define WORKER_HPP

class	Worker
{
	private:
        pthread_t    _th;
        int          _sk;

		Worker(const Worker&);
		Worker	&operator=(const Worker& op);

	public:
		Worker();
		virtual ~Worker();

        pthread_t*   get_th(void);
};

#endif
