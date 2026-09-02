/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   world_constructors.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: azarzor <azarzor@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/12/05 21:59:48 by abiri             #+#    #+#             */
/*   Updated: 2020/03/04 23:03:58 by azarzor          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "rtv1.h"

int				ft_add_light(t_xml_tag *tag, t_rtv *env)
{
	t_light	light;
	int		status;

	status = 0;
	light.center = ft_parse_vector(ft_xml_get_value(tag, "center",
				"(0,0,0)"), &status);
	light.color = ft_parse_color(ft_xml_get_value(tag, "color",
				"(255, 255, 255)"), &status);
	light.intensity = ft_parse_float(ft_xml_get_value(tag, "intensity",
				"1"), &status);
	light.radius = ft_clip_min(0, ft_parse_float(
		ft_xml_get_value(tag, "radius", "1"), &status));
	light.intensity = ft_clip_min_max(0, 1, light.intensity);
	light.light_type = (!ft_strcmp(
		ft_xml_get_value(tag, "type", "spot"), "parallel")) ? 1 : 0;
	if (light.radius > 0)
	{
		light.light_shape.center = light.center;
		light.light_shape.radius = light.radius;
	}
	light.light_direction = ft_normalise_vector(
	ft_parse_vector(ft_xml_get_value(tag, "direction", "(0, -1,0)"), &status));
	status &= ft_light_push(env, light);
	return (status);
}

int				ft_load_camera(t_xml_tag *tag, t_rtv *env)
{
	int			status;

	status = 1;
	env->cam.position = ft_parse_vector(ft_xml_get_value(tag, "position",
				"(0,0,0)"), &status);
	env->cam.look_at = ft_parse_vector(ft_xml_get_value(tag, "lookat",
				"(0,0,-1)"), &status);
	env->cam.fov = ft_parse_float(ft_xml_get_value(tag, "fov",
				"35"), &status);
	env->cam.fov = ft_clip_min(1, env->cam.fov);
	env->cam.translation = ft_parse_vector(ft_xml_get_value(tag, "translation",
				"(0,0,0)"), &status);
	return (status);
}

int				ft_parse_filter(char *filter_name)
{
	if (!ft_strcmp(filter_name, "gray scale"))
		return (1);
	if (!ft_strcmp(filter_name, "sepia"))
		return (2);
	if (!ft_strcmp(filter_name, "negatif"))
		return (3);
	if (!ft_strcmp(filter_name, "purple scale"))
		return (5);
	if (!ft_strcmp(filter_name, "yellow scale"))
		return (6);
	return (-1);
}

int				ft_parse_effects(char *filter_name)
{
	if (!ft_strcmp(filter_name, "motion blur"))
		return (1);
	if (!ft_strcmp(filter_name, "blur"))
		return (2);
	if (!ft_strcmp(filter_name, "sharpness"))
		return (3);
	if (!ft_strcmp(filter_name, "cartoon"))
		return (4);
	if (!ft_strcmp(filter_name, "outline"))
		return (5);
	return (-1);
}

int				ft_load_scene(t_xml_tag *tag, t_rtv *env)
{
	int			status;

	status = 1;
	env->scene.ambiant = ft_parse_float(ft_xml_get_value(tag, "ambiant",
				"0.4"), &status);
	env->scene.filter = ft_parse_filter(
		ft_xml_get_value(tag, "filter", "none"));
	env->scene.effect = ft_parse_effects(
		ft_xml_get_value(tag, "effect", "none"));
	env->scene.aa = ft_clip_min_max(0, 8,
				ft_parse_float(ft_xml_get_value(tag, "AA", "0"), &status));
	env->scene.dof = ft_clip_min(0, ft_parse_float(
			ft_xml_get_value(tag, "depth_of_field", "0"), &status));
	env->scene.dof_aperture = ft_clip_min(0, ft_parse_float(
			ft_xml_get_value(tag, "dof_aperture", "0.5"), &status));
	env->scene.reflection_depth = ft_parse_float(
				ft_xml_get_value(tag, "reflection_depth", "3"), &status);
	env->scene.refraction_depth = ft_parse_float(
				ft_xml_get_value(tag, "refraction_depth", "3"), &status);
	env->scene.height = ft_parse_float(
				ft_xml_get_value(tag, "resolution", "720"), &status);
	env->scene.light_samples = ft_clip_min(1, ft_parse_float(
				ft_xml_get_value(tag, "light_samples", "1"), &status));
	if (env->scene.height < 50)
		env->scene.height = 50;
	env->scene.width = (1.77777777778 * env->scene.height);
	return (status);
}
