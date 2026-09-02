/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   string_parsers.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenaiss <abenaiss@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/12/04 16:18:44 by abiri             #+#    #+#             */
/*   Updated: 2019/12/23 07:14:24 by abenaiss         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "rtv1.h"

t_vector		ft_parse_vector(char *string, int *status)
{
	t_vector	result;
	size_t		index;
	int			size;

	index = 0;
	result = (t_vector){0, 0, 0};
	if (string[0] != '(')
	{
		*status = 0;
		return (result);
	}
	result.x = ft_atof(&string[1], &size);
	index += size;
	if (string[index] != ',' && !(*status = 0))
		return (result);
	result.y = ft_atof(&string[index + 1], &size);
	index += size;
	if (string[index] != ',' && !(*status = 0))
		return (result);
	result.z = ft_atof(&string[index + 1], &size);
	index += size;
	if ((string[index] != ')' || string[index + 1] != '\0') && !(*status = 0))
		return (result);
	return (result);
}

t_coor			ft_parse_coor(char *string, int *status)
{
	t_coor		result;
	size_t		index;
	int			size;

	index = 0;
	result = (t_coor){0, 0};
	if (string[0] != '(')
	{
		*status = 0;
		return (result);
	}
	result.x = ft_atof(&string[1], &size);
	index += size;
	if (string[index] != ',' && !(*status = 0))
		return (result);
	result.y = ft_atof(&string[index + 1], &size);
	index += size;
	if ((string[index] != ')' || string[index + 1] != '\0') && !(*status = 0))
		return (result);
	return (result);
}

double			ft_parse_float(char *string, int *status)
{
	double	result;
	int		index;

	result = ft_atof(string, &index);
	if (string[index - 1])
	{
		*status = 0;
		return (result);
	}
	return (result);
}

int				ft_get_hex_byte(char *string, int *status)
{
	int		byte;
	size_t	index;
	size_t	jndex;

	byte = 0;
	index = 0;
	while (index < 2 && string[index])
	{
		jndex = 0;
		if (!ft_strchr("0123456789abcdef", ft_tolower(string[index])))
		{
			*status = 0;
			return (0);
		}
		while (jndex < 16)
		{
			if (ft_tolower(string[index]) ==
					"0123456789abcdef"[jndex])
				byte += jndex * ((index == 0) ? 16 : 1);
			jndex++;
		}
		index++;
	}
	return (byte);
}

t_color			ft_parse_color(char *string, int *status)
{
	t_vector	rgb_color;
	t_color		result;

	rgb_color = ft_parse_vector(string, status);
	if (*status)
	{
		result.r = rgb_color.x / 255;
		result.g = rgb_color.y / 255;
		result.b = rgb_color.z / 255;
		return (result);
	}
	*status = 1;
	result = ft_parse_hex(string, status);
	return (result);
}
