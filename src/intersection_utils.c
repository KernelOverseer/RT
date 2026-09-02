/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   intersection_utils.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: azarzor <azarzor@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/26 07:22:58 by abenaiss          #+#    #+#             */
/*   Updated: 2020/03/04 21:28:39 by azarzor          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "rtv1.h"

static void			ft_display_loading(t_rtv *rtv)
{
	int x;
	int y;
	int	max;

	y = rtv->scene.height - 5;
	x = 0;
	max = (double)rtv->scene.width - ((double)((rtv->render_y_offset)
	* rtv->pixel_size + (rtv->render_offset)) / (double)(rtv->pixel_size
	* rtv->pixel_size + rtv->pixel_size)) * (double)rtv->scene.width;
	max = (max == rtv->scene.width) ? 0 : max;
	while (x < max)
	{
		y = rtv->scene.height - 5;
		while (y < rtv->scene.height)
		{
			if (x >= 0 && x < rtv->scene.width &&
					y >= 0 && y < rtv->scene.height)
				rtv->mlx.img.data[(int)(y * rtv->scene.width + x)] = 0xFF00FF;
			y++;
		}
		x++;
	}
}

void				ft_render_band(t_rtv *rtv)
{
	t_color		rgb;

	rtv->column = rtv->render_y_offset;
	while (rtv->column <= rtv->scene.height)
	{
		rtv->row = rtv->min_w + rtv->render_offset;
		while (rtv->row <= rtv->max_w)
		{
			rgb = (t_color){0, 0, 0};
			if (rtv->scene.dof && rtv->options.depth_of_field)
				ft_color_best_node_dof(rtv, rgb);
			ft_color_best_node(rtv, rgb);
			rtv->row += rtv->pixel_size;
		}
		rtv->column += rtv->pixel_size;
#ifndef __EMSCRIPTEN__
		ft_display_loading(rtv);
#endif
	}
}

static void			*ft_ray_loop(void *data)
{
	ft_render_band(data);
	return (NULL);
}

#ifndef __EMSCRIPTEN__
int					ft_ray_shooter(t_rtv *rtv)
{
	pthread_t	thread[NUM_THREAD];
	t_rtv		rtv_cpy[NUM_THREAD];
	int			i;

	i = -1;
	while (++i < NUM_THREAD)
	{
		rtv_cpy[i] = *rtv;
		rtv_cpy[i].min_w = (rtv->scene.width / NUM_THREAD) * i;
		rtv_cpy[i].max_w = (rtv->scene.width / NUM_THREAD) * (i + 1);
		pthread_create(&thread[i], NULL, ft_ray_loop, &rtv_cpy[i]);
	}
	while (i--)
		pthread_join(thread[i], NULL);
	ft_update_offset(rtv);
	return (0);
}
#endif
