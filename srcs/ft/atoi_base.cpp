/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   atoi_base.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/23 08:42:44 by ngoc              #+#    #+#             */
/*   Updated: 2023/12/23 15:22:23 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstddef>

int	in_str(const char *c, const char *base)
{
	int	i;

	i = 0;
	while (*base)
	{
		if (*base == *c)
			return (i);
		base++;
		i++;
	}
	return (-1);
}

size_t	get_base(const char *base)
{
	size_t		i;
	size_t		j;
	const char	*c;

	c = base;
	i = 0;
	while (*base)
	{
		if (*base == '+' || *base == '-' || (!(*base >= 33 && *base <= 126)))
			return (0);
		j = 0;
		while (j < i)
		{
			if (c[j] == *base)
				return (0);
			j++;
		}
		i++;
		base++;
	}
	if (i <= 1)
		return (0);
	return (i);
}

namespace ft {

size_t	atoi_base(const char *str, const char *base)
{
    size_t		b;
	size_t		nbr;
	//int		sign;

	b = get_base(base);
	if (b == 0)
		return (0);
	nbr = 0;
	//sign = 1;
	while (*str == 9 || *str == 10 || *str == 11
		|| *str == 12 || *str == 13 || *str == 32)
		str++;
	while (*str == '-' || *str == '+')
	{
		//if (*str == '-')
		//	sign *= -1;
		str++;
	}
	while (*str && in_str(str, base) != -1)
	{
		nbr *= b;
		nbr += in_str(str, base);
		str++;
	}
	//return (sign * nbr);
	return (nbr);
}

}
