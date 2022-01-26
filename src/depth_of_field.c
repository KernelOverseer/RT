/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   depth_of_field.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: azarzor <azarzor@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/03/01 19:26:03 by azarzor           #+#    #+#             */
/*   Updated: 2020/03/04 21:46:07 by azarzor          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "rtv1.h"

static void				ft_offset_dof(t_rtv *rtv,
		double radius, t_vector random, t_vector focus)
{
	rtv->cam.ray_origin = ft_sub_vector(rtv->cam.position,
			ft_add_vector(ft_sub_vector(
					ft_scale_vector(rtv->cam.right, radius / 2.0),
					ft_scale_vector(rtv->cam.up, radius / 2.0)),
				ft_add_vector(
					ft_scale_vector(rtv->cam.right, radius * random.x),
					ft_scale_vector(rtv->cam.up, radius * random.y))));
	rtv->cam.ray_direction = ft_normalise_vector(
			ft_sub_vector(focus, rtv->cam.ray_origin));
}

void					ft_depth_of_field(t_rtv *rtv)
{
	t_vector			world_point;
	t_vector			random;
	t_vector			focus;
	double				radius;

	random = ft_new_vector(rand() / (float)(RAND_MAX),
			rand() / (float)(RAND_MAX), 0);
	radius = 0.5;
	world_point = ft_add_vector(rtv->cam.bottom_left,
			ft_add_vector(ft_scale_vector(rtv->cam.h_scalar,
					(rtv->column + 0.5) / rtv->scene.height),
				ft_scale_vector(rtv->cam.w_scalar,
					(rtv->row + 0.5) / rtv->scene.width)));
	rtv->cam.ray_direction = ft_sub_vector(world_point, rtv->cam.position);
	focus = ft_add_vector(rtv->cam.position,
			ft_scale_vector(rtv->cam.ray_direction, rtv->scene.dof_focus));
	rtv->cam.ray_direction = ft_normalise_vector(rtv->cam.ray_direction);
	ft_offset_dof(rtv, radius, random, focus);
}

void					ft_change_lookat(int mouse_button,
			int x, int y, t_rtv *env)
{
	if (mouse_button == 1)
	{
		ft_init_rendrering(env);
		env->column = y;
		env->row = x;
		ft_specific_ray_shoot(env);
		env->cam.look_at = env->cam.hit.position;
		ft_init_cam(&env->cam, *env);
		env->scene.dof_focus = ft_vector_size(ft_sub_vector(
					env->cam.ray_origin, env->cam.look_at));
	}
}

void					ft_toogle_dof(t_list_head *buttons, t_rtv *env)
{
	t_button *button;

	(void)buttons;
	button = ft_new_button(ft_strdup("DOF"), ft_button_toggle_boolean,
			&env->options.depth_of_field, (t_coor){0, 120});
	env->buttons.push(&env->buttons, button);
}
