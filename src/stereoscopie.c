/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   stereoscopie.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenaiss <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/02/23 02:15:51 by abenaiss          #+#    #+#             */
/*   Updated: 2020/02/23 02:15:53 by abenaiss         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "rtv1.h"

void		ft_update_offset(t_rtv *rtv)
{
	if (rtv->render_y_offset >= 0)
	{
		if (rtv->render_offset > 0)
			rtv->render_offset--;
		else
		{
			rtv->render_y_offset--;
			if (rtv->render_y_offset > 0)
				rtv->render_offset = PIXEL_SIZE;
		}
	}
}

int			*ft_add_images(int *image1, int *image2, t_rtv rtv)
{
	int		max_size;
	int		i;
	t_color	first_color;
	t_color	second_color;

	i = -1;
	max_size = rtv.scene.width * rtv.scene.height;
	while (++i < max_size)
	{
		first_color = ft_scale_colors(ft_int_to_rgb(image1[i]), 1.0 / 255.0);
		second_color = ft_scale_colors(ft_int_to_rgb(image2[i]), 1.0 / 255.0);
		image1[i] = ft_rgb_to_int(ft_add_colors(first_color, second_color));
	}
	return (image1);
}

#ifndef __EMSCRIPTEN__
int			ft_shoot_stero(t_rtv *rtv)
{
	t_cam	cam_clone;
	int		*image_clone;

	cam_clone = rtv->cam;
	image_clone = (int*)malloc(sizeof(int) *
		rtv->scene.height * rtv->scene.width);
	rtv->scene.filter = 8;
	rtv->cam.position = ft_add_vector(cam_clone.position,
		ft_scale_vector(cam_clone.right, -0.6));
	ft_init_cam(&rtv->cam, *rtv);
	ft_ray_shooter(rtv);
	rtv->scene.filter = 7;
	ft_memcpy(image_clone, rtv->mlx.img.data, 4 *
		rtv->scene.width * rtv->scene.height);
	rtv->cam.position = ft_add_vector(cam_clone.position,
		ft_scale_vector(cam_clone.right, 0.6));
	ft_init_cam(&rtv->cam, *rtv);
	ft_ray_shooter(rtv);
	rtv->mlx.img.data = ft_add_images(rtv->mlx.img.data, image_clone, *rtv);
	free(image_clone);
	rtv->cam = cam_clone;
	ft_init_cam(&rtv->cam, *rtv);
	rtv->scene.filter = 0;
	return (0);
}
#endif
