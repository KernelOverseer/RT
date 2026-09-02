/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   destroy.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abenaiss <abenaiss@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/01/16 22:15:39 by abenaiss          #+#    #+#             */
/*   Updated: 2020/01/16 22:18:31 by abenaiss         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "rtv1.h"

#ifndef __EMSCRIPTEN__
void		ft_clear_mlx(t_mlx *mlx, t_rtv *rtv)
{
	mlx_destroy_image(mlx->mlx_ptr, mlx->img.img_ptr);
	mlx_clear_window(mlx->mlx_ptr, mlx->win);
	mlx->img.img_ptr =
		mlx_new_image(mlx->mlx_ptr, rtv->scene.width, rtv->scene.height);
	mlx->img.data =
		(int*)mlx_get_data_addr(mlx->img.img_ptr, &mlx->img.bpp,
				&mlx->img.size_l, &mlx->img.endian);
}
#endif

int			ft_exit(t_rtv *rtv)
{
	(void)rtv;
	exit(0);
}
