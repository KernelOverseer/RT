/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mlx_managing.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: azarzor <azarzor@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/02/29 03:31:17 by abenaiss          #+#    #+#             */
/*   Updated: 2020/03/04 22:07:51 by abiri            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "rtv1.h"

void		ft_put_pixel(t_rtv *rtv, int color)
{
	int i;
	int j;

	i = -1;
	while (++i < rtv->render_offset)
	{
		j = -1;
		while (++j < rtv->render_y_offset)
		{
			if (rtv->row >= 0 && rtv->row < rtv->scene.width &&
				rtv->column >= 0 && rtv->column < rtv->scene.height)
				rtv->mlx.img.data[(int)
				(((int)(rtv->column - j) % rtv->scene.height)
				* rtv->scene.width + (int)(rtv->row - i)
				% rtv->scene.width)] = color;
		}
	}
	if (rtv->row >= 0 && rtv->row < rtv->scene.width &&
		rtv->column >= 0 && rtv->column < rtv->scene.height)
		rtv->mlx.img.data[
			(int)(rtv->column * rtv->scene.width + rtv->row)] = color;
}

#ifndef __EMSCRIPTEN__
static int	ft_frame_loop(void *arg)
{
	t_rtv	*rtv;

	rtv = arg;
	if (rtv->render_offset >= 0 && rtv->render_y_offset > 0)
		ft_ray_shooter(rtv);
	else if (rtv->anti_aliasing != rtv->scene.aa)
	{
		rtv->render_offset = 0;
		rtv->render_y_offset = 0;
		rtv->pixel_size = 1;
		if (rtv->options.anti_aliasing)
			rtv->anti_aliasing = rtv->scene.aa;
	}
	mlx_put_image_to_window(rtv->mlx.mlx_ptr, rtv->mlx.win,
			rtv->mlx.img.img_ptr, 0, 0);
	ft_draw_buttons(rtv);
	return (0);
}
#endif

void		ft_init_rendrering(t_rtv *rtv)
{
	rtv->anti_aliasing = 0;
	rtv->render_offset = PIXEL_SIZE;
	rtv->render_y_offset = PIXEL_SIZE;
	rtv->pixel_size = PIXEL_SIZE;
	rtv->actions.mouvement = 0;
}

#ifndef __EMSCRIPTEN__
void		ft_init_win(t_rtv *rtv)
{
	ft_init_rendrering(rtv);
	rtv->mlx.mlx_ptr = mlx_init();
	rtv->mlx.img.img_ptr = mlx_new_image(rtv->mlx.mlx_ptr,
			rtv->scene.width, rtv->scene.height);
	rtv->mlx.img.data = (int*)mlx_get_data_addr(rtv->mlx.img.img_ptr,
			&rtv->mlx.img.bpp, &rtv->mlx.img.size_l, &rtv->mlx.img.endian);
	rtv->mlx.win = mlx_new_window(rtv->mlx.mlx_ptr, rtv->scene.width,
		rtv->scene.height, "RTV1");
	rtv->mlx.img.height = rtv->scene.height;
	rtv->mlx.img.width = rtv->scene.width;
	mlx_hook(rtv->mlx.win, 2, 0, &ft_key_stroke, rtv);
	mlx_hook(rtv->mlx.win, 17, 1, (*ft_exit), rtv);
	mlx_mouse_hook(rtv->mlx.win, ft_click_buttons, rtv);
	mlx_loop_hook(rtv->mlx.mlx_ptr, &ft_frame_loop, rtv);
	mlx_loop(rtv->mlx.mlx_ptr);
}
#endif
