/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mlx.h                                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: KernelOverseer <marvin@42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+            */
/*   Created: 2026/09/02 by KernelOverseer        #+#    #+#              */
/*   Updated: 2026/09/02 by KernelOverseer       ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

/*
**	Minimal minilibx stand-in for the WebAssembly build.
**	The web driver replaces the mlx window entirely (canvas + JS events),
**	so only the declarations rtv1.h needs to parse are provided here.
**	No mlx function is ever called in an __EMSCRIPTEN__ build.
*/

#ifndef MLX_H
# define MLX_H

void	*mlx_init(void);
void	*mlx_new_window(void *mlx_ptr, int size_x, int size_y, char *title);
void	*mlx_new_image(void *mlx_ptr, int width, int height);
char	*mlx_get_data_addr(void *img_ptr, int *bits_per_pixel,
			int *size_line, int *endian);
int		mlx_put_image_to_window(void *mlx_ptr, void *win_ptr, void *img_ptr,
			int x, int y);
int		mlx_string_put(void *mlx_ptr, void *win_ptr, int x, int y, int color,
			char *string);
int		mlx_pixel_put(void *mlx_ptr, void *win_ptr, int x, int y, int color);
int		mlx_clear_window(void *mlx_ptr, void *win_ptr);
int		mlx_destroy_image(void *mlx_ptr, void *img_ptr);
int		mlx_hook(void *win_ptr, int x_event, int x_mask,
			int (*funct)(), void *param);
int		mlx_mouse_hook(void *win_ptr, int (*funct)(), void *param);
int		mlx_loop_hook(void *mlx_ptr, int (*funct)(), void *param);
int		mlx_loop(void *mlx_ptr);

#endif
