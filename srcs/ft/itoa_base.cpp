/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   itoa_base.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ngoc <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/23 08:47:38 by ngoc              #+#    #+#             */
/*   Updated: 2023/12/23 15:23:52 by ngoc             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>

unsigned int	get_base(char *base)
{
	unsigned int	i;
	unsigned int	j;
	char			*c;

	c = base;
	i = 0;
	while (*base != '\0')
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

void	get_str(unsigned int nbr, char *base, unsigned int b, std::string& s)
{
    char    o[2];

    o[1] = 0;
	if (nbr > (b - 1))
		get_str((nbr - nbr % b) / b, base, b, s);
    o[0] = base[nbr % b];
    s += o;
	//write(1, &base[nbr % b], 1);
}

namespace ft {

std::string	itoa_base(int nbr, char *base)
{
	unsigned int	n;
	int				b;
    std::string     s = "";

	b = get_base(base);
	if (b == 0)
		return (s);
	if (nbr < 0)
	{
		//write(1, "-", 1);
        s += "-";
		n = -nbr;
	}
	else
	{
		n = nbr;
	}
	get_str(n, base, b, s);
    return (s);
}

}
