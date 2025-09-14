/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strcpy.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vflores <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/23 16:54:49 by vflores           #+#    #+#             */
/*   Updated: 2024/09/30 12:21:23 by vflores          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

char	*ft_strcpy(char *dest, char *src)
{
	int	i;

	i = 0;
	while (src[i] != '\0')
	{
		dest[i] = src[i];
		i++;
	}
	dest[i] = '\0';
	return (dest);
}

/*
#include <unistd.h>
#include <stdio.h>

int	main(void)
{
	char	mot[] = "Hola";
	char destin[5];
	
	printf("Primero: %s \n", destin);
	ft_strcpy(destin, mot);
	printf("Despues: %s \n", destin);
	return (0);
}
*/
